#include "formula_pointer_tree.h"

using namespace spiritsaway::formula_tree::runtime;

std::vector<attr_value_pair> formula_pointer_tree::process_update_queue()
{
	std::vector<attr_value_pair> result;
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
		if (!(cur_top->cacl_type == node_type::root))
		{
			result.emplace_back(cur_top->name, cur_top->value);
		}
	}
	nodes_in_queue.clear();
	return result;
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
	if (cur_iter == nodes_in_queue.end())
	{
		return false;
	}
	nodes_in_queue.insert(new_node);
	return true;
}
formula_pointer_tree::formula_pointer_tree(const formula_desc_flat& flat_nodes_info, const std::vector<attr_value_pair>& init_values)
	: formula_tree_interface(flat_nodes_info, init_values)
{
	all_nodes.reserve(flat_nodes_info.flat_nodes.size());
	// create all nodes
	for (auto& one_node : flat_nodes_info.flat_nodes)
	{
		auto cur_pointer_node = cacl_pointer_node(this, one_node.name, one_node.type);
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
	// init values
	update_attr_batch(init_values);
}
