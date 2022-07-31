#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <unordered_map>
#include <string_view>
#include <cstdlib>
#include <fstream>
#include <filesystem>
#include <unordered_set>
#include <magic_enum.hpp>
#include <any_container/encode.h>
#include "formula_desc.h"

using namespace spiritsaway::formula_tree::runtime;

enum class op_priority
{
	comma,
	add_minus,
	multiply_devide,
	func_paren,
};

enum class tree_node_type
{
	value,
	variable,
	op,
	func
};
enum class token_type
{
	invalid,
	value,
	math_op,
	var,
};


struct tree_node
{
	std::vector<tree_node*> children;
	std::string token;
	double value;
	tree_node_type m_type;
	std::uint32_t node_idx;
	bool convert_args(const std::unordered_map<std::string, std::uint8_t>& func_arg_num)
	{
		auto cur_iter = func_arg_num.find(token);
		if (cur_iter == func_arg_num.end())
		{
			return false;
		}
		auto cur_top = children[0];
		children.clear();
		while (true)
		{
			if (cur_top->m_type == tree_node_type::op && cur_top->token == ",")
			{
				children.push_back(cur_top->children[0]);
				auto pre_top = cur_top;
				cur_top = cur_top->children[1];
				delete(pre_top);
			}
			else
			{
				children.push_back(cur_top);
				break;
			}
		}
		if (cur_iter->second == 0)
		{
			return !children.empty();
		}
		else
		{
			return children.size() == cur_iter->second;
		}
	}
	std::string to_string() const
	{
		switch (m_type)
		{
		case tree_node_type::op:
		{
			std::string result;
			result += std::string("(") + children[0]->to_string();
			for (int i = 1; i < children.size(); i++)
			{
				result += std::string(token) + children[i]->to_string();
			}
			result += std::string(")");
			return result;
		}

		case tree_node_type::value:
		case tree_node_type::variable:
			return std::string(token);
		case tree_node_type::func:
		{
			std::string result;
			result += std::string(token);
			result += "(";
			for (int i = 0; i + 1 < children.size(); i++)
			{
				result += children[i]->to_string();
				result += ",";
			}
			result += children.back()->to_string();
			result += ")";
			return result;
		}
		default:
			return "invalid";
		}
	}
	void to_tree(const std::unordered_set<std::string>& input_nodes, cacl_tree& result) const
	{
		cacl_node_desc cur_desc;
		cur_desc.idx = node_idx;
		cur_desc.value = 0;
		for (const auto& one_child : children)
		{
			cur_desc.children.push_back(one_child->node_idx);
			one_child->to_tree(input_nodes, result);
		}
		cur_desc.name = token;
		switch (m_type)
		{
		case tree_node_type::value:
		{
			cur_desc.value = value;
			cur_desc.type = node_type::literal;
		}
			break;
		case tree_node_type::variable:
		{
			if (input_nodes.count(std::string(token)) == 1)
			{
				cur_desc.type = node_type::input;
			}
			else
			{
				cur_desc.type = node_type::import;
			}
		}
			break;
		case tree_node_type::op:
		{
			if (token[0] == '+')
			{
				cur_desc.type = node_type::add;
			}
			else if (token[0] == '-')
			{
				cur_desc.type = node_type::dec;
			}
			else if (token[0] == '/')
			{
				cur_desc.type = node_type::div;
			}
			else if (token[0] == '*')
			{
				cur_desc.type = node_type::mul;
			}

		}
			break;
		case tree_node_type::func:
		{
			auto opt_type = magic_enum::enum_cast<node_type>(token);
			cur_desc.type = opt_type.value();
		}
			break;
		default:
			break;
		}
		result.nodes[cur_desc.idx] = cur_desc;

	}
	void compact()
	{
		// 把多个连续的add 和mul 替换为一个 这样计算的时候效率会高很多
		if (m_type != tree_node_type::op || (token[0] != '+' && token[0] != '*') || children.size() != 2)
		{
			for (auto one_child : children)
			{
				one_child->compact();
			}
		}
		else
		{
			std::vector<tree_node*> temp_parents;
			tree_node* temp_node = children[0];
			children[0] = children[1];
			children.pop_back();
			while (temp_node->token == token)
			{
				children.push_back(temp_node->children[1]);
				temp_parents.push_back(temp_node);
				temp_node = temp_node->children[0];

			}
			children.push_back(temp_node);
			std::reverse(children.begin(), children.end());
			for (auto one_parent : temp_parents)
			{
				delete one_parent;
			}
			temp_parents.clear();
			for (auto one_child : children)
			{
				one_child->compact();
			}
		}
	}
};
std::string_view remove_blank(std::string_view remain_str)
{
	for (std::uint32_t i = 0; i < remain_str.size(); i++)
	{
		if (remain_str[i] == ' ' || remain_str[i] == '\t')
		{
			continue;
		}
		return remain_str.substr(i);
	}
	return remain_str;
}



// parse double value
std::uint32_t get_num_token(std::string_view remain_str)
{
	char* number_end = nullptr;
	const char* number_begin = remain_str.data();
	auto cur_value = std::strtold(number_begin, &number_end);
	return number_end - number_begin;
}

std::uint32_t get_variable_token(std::string_view remain_str)
{
	for (int i = 0; i < remain_str.size(); i++)
	{
		auto cur_char = remain_str[i];
		if (cur_char >= 'a' && cur_char <= 'z')
		{
			continue;
		}
		if (cur_char >= 'A' && cur_char <= 'Z')
		{
			continue;
		}
		if (cur_char >= '0' && cur_char <= '9')
		{
			continue;
		}
		if (cur_char == '_')
		{
			continue;
		}
		return i;
	}
	return remain_str.size();
}

std::uint32_t get_op_token(std::string_view remain_str)
{
	auto c = remain_str[0];
	if (c == '+' || c == '-' || c == '*' || c == '/' || c == ',' || c == '(' || c == ')')
	{
		return 1;
	}
	return 0;
}

std::pair<std::string_view, token_type>  get_next_token(std::string_view remain_str)
{
	std::uint32_t result = 0;
	result = get_num_token(remain_str);
	if (result)
	{
		return std::make_pair(remain_str.substr(0, result), token_type::value);
	}

	result = get_op_token(remain_str);
	if (result)
	{
		return std::make_pair(remain_str.substr(0, result), token_type::math_op);
	}

	result = get_variable_token(remain_str);
	if (result)
	{
		return std::make_pair(remain_str.substr(0, result), token_type::var);
	}
	return std::make_pair(std::string_view{}, token_type::invalid);
}

std::vector< std::pair<std::string_view, token_type>> split_tokens(std::string_view origin_str)
{
	std::vector< std::pair<std::string_view, token_type>> result;
	std::string_view remain_str = remove_blank(origin_str);

	while (!remain_str.empty())
	{
		auto temp_token = get_next_token(remain_str);
		result.push_back(temp_token);
		if (temp_token.second == token_type::invalid)
		{
			return {};
		}
		if (temp_token.first.size() == remain_str.size())
		{
			break;
		}
		remain_str = remove_blank(remain_str.substr(temp_token.first.size()));
	}
	return result;
}


std::string_view strip_token(std::string_view input_str)
{
	auto cur_token_vec = split_tokens(input_str);
	if (cur_token_vec.size() != 1)
	{
		return {};
	}
	if (cur_token_vec[0].second != token_type::var)
	{
		return {};
	}
	return cur_token_vec[0].first;
}

tree_node* create_math_tree(const std::string& input, const std::unordered_map<std::string, std::uint8_t>& func_arg_num)
{
	std::unordered_map<std::string_view, op_priority> op_priority_map = {
		{"(", op_priority::func_paren},
		{")", op_priority::func_paren},
		{"*", op_priority::multiply_devide},
		{"/", op_priority::multiply_devide},
		{"+", op_priority::add_minus},
		{"-", op_priority::add_minus},
		{",", op_priority::comma},
	};
	std::uint32_t node_idx_counter = 1;
	auto tokens = split_tokens(input);
	if (tokens.empty() || tokens.back().second == token_type::invalid)
	{
		return nullptr;
	}
	tokens.push_back(std::make_pair(",", token_type::math_op));
	std::stack<std::pair<op_priority, std::string_view>> m_op_stack;
	std::stack<std::pair<std::string_view, tree_node*>> m_value_stack;
	bool fail = false;
	for (std::uint64_t i = 0; i < tokens.size(); i++)
	{
		if (fail)
		{
			break;
		}
		const auto& one_token = tokens[i];
		switch (one_token.second)
		{
		case token_type::value:
		{
			auto new_node = new tree_node();
			new_node->token = one_token.first;
			new_node->m_type = tree_node_type::value;
			char* number_end = nullptr;
			const char* number_begin = one_token.first.data();
			auto cur_value = std::strtold(number_begin, &number_end);
			new_node->value = cur_value;
			new_node->node_idx = node_idx_counter++;
			m_value_stack.emplace(std::make_pair(one_token.first, new_node));
			break;
		}
		case token_type::var:
		{
			auto cur_iter = func_arg_num.find(std::string(one_token.first));
			if (cur_iter == func_arg_num.end())
			{
				auto new_node = new tree_node();
				new_node->token = one_token.first;
				new_node->m_type = tree_node_type::variable;
				new_node->node_idx = node_idx_counter++;
				m_value_stack.emplace(std::make_pair(one_token.first, new_node));
				break;
			}
			else
			{
				if (i + 1 == tokens.size())
				{
					fail = true;
					break;
				}
				const auto& next_token = tokens[i + 1];
				if (next_token.second != token_type::math_op || next_token.first != "(")
				{
					fail = true;
					break;
				}
				m_op_stack.emplace(std::make_pair(op_priority::func_paren, one_token.first));
				i += 1;
				break;
			}
		}
		case token_type::math_op:
		{
			op_priority cur_pri = op_priority_map[one_token.first];
			if (one_token.first == ")")
			{
				if (m_op_stack.empty())
				{
					std::cout << "op stack empty while ) meet" << std::endl;
					fail = true;
					break;
				}
				fail = true;
				while (!m_op_stack.empty())
				{
					auto cur_top = m_op_stack.top();
					m_op_stack.pop();
					if (cur_top.first == op_priority::func_paren)
					{
						if (cur_top.second != "(")
						{
							auto new_node = new tree_node();
							new_node->token = cur_top.second;
							new_node->m_type = tree_node_type::func;
							new_node->node_idx = node_idx_counter++;
							new_node->children.push_back(m_value_stack.top().second);
							if (!new_node->convert_args(func_arg_num))
							{
								std::cout << "func " << new_node->to_string() << " called with invalid arg num" << std::endl;
								break;
							}
							m_value_stack.pop();
							m_value_stack.emplace(std::make_pair(cur_top.second, new_node));
						}
						fail = false;
						break;
					}
					else
					{
						if (m_value_stack.size() < 2)
						{
							std::cout << "value stack should has at least 2 node when meet " << cur_top.second << std::endl;
							fail = true;
							break;
						}
						auto new_node = new tree_node();
						new_node->token = cur_top.second;
						new_node->m_type = tree_node_type::op;
						new_node->node_idx = node_idx_counter++;
						new_node->children.push_back(m_value_stack.top().second);
						m_value_stack.pop();
						new_node->children.push_back(m_value_stack.top().second);
						m_value_stack.pop();
						std::reverse(new_node->children.begin(), new_node->children.end());
						m_value_stack.emplace(std::make_pair(cur_top.second, new_node));
					}
				}
			}
			else
			{
				while (!m_op_stack.empty())
				{
					auto cur_top = m_op_stack.top();
					if (int(cur_top.first) >= int(cur_pri) && cur_top.first != op_priority::func_paren)
					{
						m_op_stack.pop();
						if (m_value_stack.size() < 2)
						{
							std::cout << "value stack should has at least 2 node when meet " << cur_top.second << std::endl;
							fail = true;
							break;
						}
						auto new_node = new tree_node();
						new_node->token = cur_top.second;
						new_node->m_type = tree_node_type::op;
						new_node->node_idx = node_idx_counter++;
						new_node->children.push_back(m_value_stack.top().second);
						m_value_stack.pop();
						new_node->children.push_back(m_value_stack.top().second);
						m_value_stack.pop();
						std::reverse(new_node->children.begin(), new_node->children.end());
						m_value_stack.emplace(std::make_pair(cur_top.second, new_node));
					}
					else
					{
						break;
					}
				}
				m_op_stack.emplace(std::make_pair(cur_pri, one_token.first));
			}
			break;

		}
		default:
			fail = true;
			break;
		}

	}
	if (fail && m_value_stack.size() != 1)
	{
		return nullptr;
	}
	else
	{
		m_value_stack.top().second->compact();
		return m_value_stack.top().second;
	}
}

void simple_test()
{
	std::vector<std::string> test_formulas = {
		"1",
		"-1",
		"-1 + 2",
		"1 + 2 + 3",
		"(1+ 2)",
		"1 + sin(2)",
		"sin(1 + sin(2) * 3)",
		"sin(2 * sin(2) + 3)",
		"1 + add(2, 3)",
		"1 - 2 * 3 + add(2, 3)",
		"1* 2 - 3 + add(2, 3)",
		"1 * var_1 - var_2 * 3"
	};
	std::unordered_map<std::string, std::uint8_t> func_arg_num = {
		{"sin", 1},
		{"add", 2}
	};
	for (const auto& one_str : test_formulas)
	{
		auto cur_formula = create_math_tree(one_str, func_arg_num);
		if (cur_formula)
		{
			std::cout << "parse suc" << std::endl;
			std::cout << one_str << std::endl;
			std::cout << cur_formula->to_string() << std::endl;

		}
		else
		{
			std::cout << "fail to parse: " << one_str << std::endl;
		}
	}
}

void dump_cpp_formula(const std::string& input_file_path, const std::string& output_folder)
{
	std::ifstream fsm(input_file_path);
	std::string temp_line;
	std::unordered_set<std::string> input_vars;
	std::unordered_map<std::string, std::uint8_t> input_funcs;
	std::unordered_map<std::string, tree_node*> output_vars;
	while (std::getline(fsm, temp_line))
	{
		if (temp_line.empty())
		{
			continue;
		}
		auto end_iter = temp_line.find(";");
		if (end_iter == std::string::npos)
		{
			continue;
		}
		temp_line = temp_line.substr(0, end_iter);
		auto assign_iter = temp_line.find("=");
		if (assign_iter == std::string::npos)
		{
			// parse function
			std::string function_prefix = "double ";
			auto function_begin_iter = temp_line.find(function_prefix);
			if (function_begin_iter == std::string::npos)
			{
				std::cerr << "invalid function line " << temp_line << std::endl;
				continue;
			}
			function_begin_iter += function_prefix.size();
			auto function_end_iter = temp_line.find('(', function_begin_iter);
			if (function_end_iter == std::string::npos)
			{
				std::cerr << "invalid function line " << temp_line << std::endl;
				continue;
			}
			auto cur_func_name = temp_line.substr(function_begin_iter, function_end_iter - function_begin_iter);
			cur_func_name = strip_token(cur_func_name);
			if (cur_func_name.empty())
			{
				std::cerr << "invalid function line " << temp_line << std::endl;
				continue;
			}
			int func_arg_num = 0;
			function_end_iter = temp_line.find(function_prefix, function_end_iter);
			while (function_end_iter != std::string::npos)
			{
				func_arg_num++;
				function_end_iter += function_prefix.size();
				function_end_iter = temp_line.find(function_prefix, function_end_iter);
			}
			input_funcs[cur_func_name] = func_arg_num;
		}
		else
		{
			std::string var_prefix = "double ";
			auto var_begin_iter = temp_line.find(var_prefix);
			if (var_begin_iter == std::string::npos)
			{
				std::cerr << "invalid variable declear " << temp_line << std::endl;
				continue;
			}
			var_begin_iter += var_prefix.size();
			std::string cur_var_name = temp_line.substr(var_begin_iter, assign_iter - var_begin_iter - 1);
			cur_var_name = strip_token(cur_var_name);
			if (input_vars.find(cur_var_name) != input_vars.end() || output_vars.find(cur_var_name) != output_vars.end())
			{
				std::cerr << "duplicated var name " << cur_var_name << std::endl;
				continue;
			}
			auto remain_str_after_assign = temp_line.substr(assign_iter + 1);
			auto remain_tokens = split_tokens(remain_str_after_assign);
			if (remain_tokens.size() == 0)
			{
				std::cerr << "invalid  var declear " << temp_line << std::endl;
				continue;
			}
			if (remain_tokens.size() == 1)
			{
				char* number_end = nullptr;
				const char* number_begin = remain_tokens[0].first.data();
				auto cur_value = std::strtold(number_begin, &number_end);
				if (number_end == number_begin)
				{
					std::cerr << "invalid  input declear " << temp_line << std::endl;
					continue;
				}
				input_vars.insert(cur_var_name);
				std::cout << "get input var " << cur_var_name << " with value " << cur_value << std::endl;
			}
			else
			{
				auto cur_formula = create_math_tree(remain_str_after_assign, input_funcs);
				if (!cur_formula)
				{
					std::cerr << "invalid output declear " << temp_line << std::endl;
					continue;
				}
				output_vars[cur_var_name] = cur_formula;
				std::cout << "get output var " << cur_var_name << " with formula " << cur_formula->to_string() << std::endl;
				cacl_tree cur_formula_tree;
				cacl_node_desc cur_root_node;
				cur_root_node.idx = 0;
				cur_root_node.type = node_type::root;
				cur_root_node.children.push_back(cur_formula->node_idx);
				cur_root_node.value = 0;
				cur_formula_tree.nodes[0] = cur_root_node;
				cur_formula->to_tree(input_vars, cur_formula_tree);
				std::string dest_path = output_folder + "/" + cur_var_name + ".json";
				std::ofstream dest_of(dest_path);
				json final_json;
				std::vector<cacl_node_desc> all_nodes;
				for (const auto& one_node : cur_formula_tree.nodes)
				{
					all_nodes.push_back(one_node.second);
				}
				final_json["nodes"] = spiritsaway::serialize::encode(all_nodes);
				final_json["name"] = cur_var_name + ".json";
				final_json["extra"] = json::object_t();
				time_t rawtime;
				struct tm* timeinfo;
				char buffer[80];

				time(&rawtime);
				timeinfo = localtime(&rawtime);

				strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", timeinfo);

				final_json["signature"] = std::string(buffer);
				dest_of << final_json.dump(4) << std::endl;

			}
		}

	}
}
int main(int argc , const char** argv)
{
	if (argc != 3)
	{
		std::cerr << "need  args (input_file_path dest_folder_path) to run" << std::endl;
		return 1;
	}
	std::string input_file_path = argv[1];
	std::string dest_folder_path = argv[2];
	// std::cout << std::filesystem::current_path() << std::endl;
	// simple_test();
	dump_cpp_formula(input_file_path, dest_folder_path);
	return 1;

}