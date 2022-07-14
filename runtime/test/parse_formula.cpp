#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <unordered_map>
#include <string_view>
#include <cstdlib>
#include <fstream>
#include <filesystem>

enum class op_priority
{
	comma,
	add_minus,
	multiply_devide,
	func_paren,
};

enum class node_type
{
	value,
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
	node_type m_type;
	bool convert_args(const std::unordered_map<std::string, std::int8_t>& func_arg_num)
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
			if (cur_top->m_type == node_type::op && cur_top->token == ",")
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
		if (cur_iter->second < 0)
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
		case node_type::op:
			return std::string("(") + children[0]->to_string() + std::string(token) + children[1]->to_string() + std::string(")");
		case node_type::value:
			return std::string(token);
		case node_type::func:
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

tree_node* create_math_tree(const std::string& input, const std::unordered_map<std::string, std::int8_t>& func_arg_num)
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
			new_node->m_type = node_type::value;
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
				new_node->m_type = node_type::value;
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
							new_node->m_type = node_type::func;
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
						new_node->m_type = node_type::op;
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
						new_node->m_type = node_type::op;
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
		return m_value_stack.top().second;
	}
}

void simple_test()
{
	std::vector<std::string> test_formulas = {
		"1",
		"-1",
		"-1 + 2",
		"1 + 2",
		"(1+ 2)",
		"1 + sin(2)",
		"sin(1 + sin(2) * 3)",
		"sin(2 * sin(2) + 3)",
		"1 + add(2, 3)",
		"1 - 2 * 3 + add(2, 3)",
		"1* 2 - 3 + add(2, 3)",
		"1 * var_1 - var_2 * 3"
	};
	std::unordered_map<std::string, std::int8_t> func_arg_num = {
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

void test_cpp_file(const std::string& file_path)
{
	std::ifstream fsm(file_path);
	std::string temp_line;
	std::unordered_map<std::string, double> input_vars;
	std::unordered_map<std::string, std::int8_t> input_funcs;
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
			auto cur_func_name = temp_line.substr(function_begin_iter, function_end_iter - function_begin_iter - 1);
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
				input_vars[cur_var_name] = cur_value;
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
			}
		}

	}
}
int main()
{
	std::cout << std::filesystem::current_path() << std::endl;
	// simple_test();
	test_cpp_file("../../data/dota2.cpp");
	return 1;

}