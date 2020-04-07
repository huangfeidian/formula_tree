#include "formula_pointer_tree.h"
#include <iostream>
using namespace spiritsaway::formula_tree::runtime;

std::vector<attr_value_pair> formula_pointer_tree::process_update_queue()
{
	std::vector<attr_value_pair> result;
	std::unordered_set<std::string> reached_name;
	while (!update_queue.empty())
	{
		auto cur_top = update_queue.top();
		update_queue.pop();
		if (cur_top->update())
		{
			for (auto one_parent : cur_top->parents)
			{
				add_node_to_update_queue(one_parent);
			}
		}

		if (cur_top->cacl_type == node_type::root)
		{
			result.emplace_back(cur_top->name, cur_top->value);
			if (debug_on)
			{
				cur_top->pretty_print_value(reached_name);
			}
		}
	}
	nodes_in_queue.clear();
	return result;
}
void formula_pointer_tree::set_debug(bool in_debug_on)
{
	debug_on = in_debug_on;
}
formula_pointer_tree::~formula_pointer_tree()
{
	return;
}
std::optional<double> formula_pointer_tree::get_attr_value(const std::string& attr_name) const
{
	auto cur_iter = name_to_nodes.find(attr_name);
	if (cur_iter == name_to_nodes.end())
	{
		return {};
	}
	else
	{
		return cur_iter->second->value;
	}
}
std::vector<attr_value_pair> formula_pointer_tree::update_attr(const std::string& name, double value)
{
	std::vector<attr_value_pair> args;
	args.emplace_back(name, value);
	return update_attr_batch(args);
}
std::vector<attr_value_pair> formula_pointer_tree::update_attr_batch(const std::vector<attr_value_pair>& input_attrs)
{

	for (auto one_attr : input_attrs)
	{
		auto cur_iter = name_to_nodes.find(one_attr.first);
		if (cur_iter == name_to_nodes.end())
		{
			continue;
		}
		cur_iter->second->update_value(one_attr.second);
	}
	return process_update_queue();
}
bool formula_pointer_tree::add_node_to_update_queue(cacl_pointer_node* new_node)
{
	auto cur_iter = nodes_in_queue.find(new_node);
	if (cur_iter != nodes_in_queue.end())
	{
		return false;
	}
	nodes_in_queue.insert(new_node);
	update_queue.push(new_node);
	return true;
}
formula_pointer_tree::formula_pointer_tree(const formula_desc_flat& flat_nodes_info)
	: formula_tree_interface()
{
	std::uint32_t name_idx = 0;
	all_nodes.reserve(flat_nodes_info.flat_nodes.size());
	// create all nodes
	for (auto& one_node : flat_nodes_info.flat_nodes)
	{
		auto cur_node_name = one_node.name;
		if (cur_node_name.empty())
		{
			cur_node_name = "T-" + std::to_string(name_idx++);
		}
		auto cur_pointer_node = cacl_pointer_node(this, cur_node_name, one_node.type);
		all_nodes.push_back(cur_pointer_node);
	}
	// map names to node pointer
	auto node_begin_pointer = all_nodes.data();
	for (const auto& [k, v] : flat_nodes_info.node_indexes)
	{
		name_to_nodes[k] = node_begin_pointer + v;
	}
	std::unordered_map<cacl_pointer_node*, std::uint32_t> node_child_count;
	std::deque<cacl_pointer_node*> height_queue;
	// replace import/input leaf nodes with mapped node pointer
	for (auto& one_node : flat_nodes_info.flat_nodes)
	{
		auto& cur_node = all_nodes[one_node.idx];
		auto cur_child_size = one_node.children.size();
		if (cur_child_size)
		{
			node_child_count[node_begin_pointer + one_node.idx] = cur_child_size;
		}
		else
		{
			height_queue.push_back(node_begin_pointer  + one_node.idx);
		}
		for (auto one_child : one_node.children)
		{
			auto& cur_child_name = flat_nodes_info.flat_nodes[one_child].name;
			if (cur_child_name.empty())
			{
				// for non leaf/ literal nodes
				cur_node.add_child(node_begin_pointer + one_child);
			}
			else
			{
				// for import input children nodes
				cur_node.add_child(name_to_nodes[cur_child_name]);
			}
		}
	}
	// cacl the height
	while (!height_queue.empty())
	{
		auto cur_node = height_queue.front();
		height_queue.pop_front();
		for (auto one_parent : cur_node->parents)
		{
			one_parent->height = std::max(one_parent->height, cur_node->height + 1);

			auto cur_child_count = node_child_count[one_parent]--;
			if (cur_child_count == 1)
			{
				height_queue.push_back(one_parent);
			}
		}
	}

}
formula_pointer_tree::formula_pointer_tree()
{

}
formula_pointer_tree* formula_pointer_tree::clone() const
{
	auto result = new formula_pointer_tree();
	result->all_nodes = all_nodes;
	result->name_to_nodes = name_to_nodes;
	auto pre_vec_begin = all_nodes.data();
	auto cur_vec_begin = result->all_nodes.data();
	auto pointer_diff = result->all_nodes.data() - all_nodes.data();
	for (auto&[name, pointer] : result->name_to_nodes)
	{
		pointer = pointer - pre_vec_begin + cur_vec_begin;
	}
	for (auto& one_node : result->all_nodes)
	{
		one_node.tree = result;
		for (auto& pointer : one_node.children)
		{
			pointer = pointer - pre_vec_begin + cur_vec_begin;
		}
		for (auto& pointer : one_node.parents)
		{
			pointer = pointer - pre_vec_begin + cur_vec_begin;
		}
	}
	return result;
}

void formula_pointer_tree::pretty_print() const
{
	std::unordered_set<std::string> reached_names;
	for (auto[k, v] : name_to_nodes)
	{
		if (v->cacl_type == node_type::root)
		{
			if (reached_names.count(k) == 0)
			{
				auto pre_result = v->pretty_print(reached_names);
				if (pre_result != k)
				{
					std::cout << k << "=" << pre_result << std::endl;

				}
			}
			
		}
	}
}


void formula_pointer_tree::pretty_print_value() const
{
	std::unordered_set<std::string> reached_names;
	for (auto[k, v] : name_to_nodes)
	{
		if (v->cacl_type == node_type::root)
		{
			if (reached_names.count(k) == 0)
			{
				auto pre_result = v->pretty_print_value(reached_names);
				if (pre_result.find(k + "(") != 0)
				{
					std::cout << k << "=" << pre_result << std::endl;
				}
				
			}
		}
	}
}


formula_tree_mgr::formula_tree_mgr()
{

}
formula_tree_mgr& formula_tree_mgr::instance()
{
	static formula_tree_mgr the_one;
	return the_one;
}
formula_pointer_tree* formula_tree_mgr::load_formula_group(const std::string& formula_group_name, const formula_desc& output_node)
{
	auto cur_iter = named_formulas.find(formula_group_name);
	if (cur_iter != named_formulas.end())
	{
		return cur_iter->second->clone();
	}
	else
	{
		auto cur_flat_info = formula_desc_flat(output_node);
		auto cur_tree = new formula_pointer_tree(cur_flat_info);
		named_formulas[formula_group_name] = cur_tree;
		return cur_tree->clone();
	}
}