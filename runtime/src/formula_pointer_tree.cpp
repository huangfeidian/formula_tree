#include "formula_pointer_tree.h"
#include <iostream>
using namespace spiritsaway::formula_tree::runtime;

formula_value_tree::formula_value_tree(const formula_structure_tree& in_node_tree)
	: m_node_tree(in_node_tree)
	, m_node_values(in_node_tree.nodes().size(), 1.0)
	, m_node_in_queue_flag(in_node_tree.nodes().size(), 0)
{

}
std::vector<attr_value_pair> formula_value_tree::process_update_queue()
{
	std::vector<attr_value_pair> result;
	std::unordered_set<std::string> reached_name;
	while (!update_queue.empty())
	{
		auto cur_top = update_queue.top();
		update_queue.pop();
		if (cur_top->update(m_node_values))
		{
			for (auto one_parent : cur_top->parents)
			{
				add_node_to_update_queue(one_parent);
			}
		}

		if (cur_top->cacl_type == node_type::root)
		{
			result.emplace_back(cur_top->name, m_node_values[cur_top->m_node_idx]);
			if (m_debug_on)
			{
				cur_top->pretty_print_value(m_node_values, reached_name);
			}
		}
	}
	for (const auto& one_idx : m_in_queue_nodes)
	{
		m_node_in_queue_flag[one_idx] = 0;
	}
	m_in_queue_nodes.clear();
	return result;
}
void formula_value_tree::set_debug(bool in_debug_on)
{
	m_debug_on = in_debug_on;
}
formula_value_tree::~formula_value_tree()
{
	return;
}
std::optional<double> formula_value_tree::get_attr_value(const std::string& attr_name) const
{
	const auto& name_to_idx = m_node_tree.name_to_idx();
	auto cur_iter = name_to_idx.find(attr_name);
	if (cur_iter == name_to_idx.end())
	{
		return {};
	}
	else
	{
		return m_node_values[cur_iter->second];
	}
}
std::vector<attr_value_pair> formula_value_tree::update_attr(const std::string& name, double value)
{
	std::vector<attr_value_pair> args;
	args.emplace_back(name, value);
	return update_attr_batch(args);
}
std::vector<attr_value_pair> formula_value_tree::update_attr_batch(const std::vector<attr_value_pair>& input_attrs)
{
	const auto& name_to_idx = m_node_tree.name_to_idx();
	for (auto one_attr : input_attrs)
	{
		auto cur_iter = name_to_idx.find(one_attr.first);
		if (cur_iter == name_to_idx.end())
		{
			continue;
		}
		m_node_tree.nodes()[cur_iter->second].update_value(this, m_node_values, one_attr.second);
	}
	return process_update_queue();
}
bool formula_value_tree::add_node_to_update_queue(cacl_pointer_node* new_node)
{
	if (m_node_in_queue_flag[new_node->node_idx()])
	{
		return false;
	}
	m_node_in_queue_flag[new_node->node_idx()] = 1;
	m_in_queue_nodes.push_back(std::uint32_t(new_node->node_idx()));
	update_queue.push(new_node);
	return true;
}
formula_structure_tree::formula_structure_tree(const formula_desc_flat& flat_nodes_info)
{
	std::uint32_t name_idx = 0;
	m_nodes.reserve(flat_nodes_info.flat_nodes.size());
	// create all nodes
	for (auto& one_node : flat_nodes_info.flat_nodes)
	{
		auto cur_node_name = one_node.name;
		if (cur_node_name.empty())
		{
			cur_node_name = "T-" + std::to_string(name_idx++);
		}
		auto cur_pointer_node = cacl_pointer_node(this, m_nodes.size(), cur_node_name, one_node.type);
		m_nodes.push_back(cur_pointer_node);
	}
	// map names to node pointer
	auto node_begin_pointer = m_nodes.data();
	for (const auto& [k, v] : flat_nodes_info.node_indexes)
	{
		m_name_to_idx[k] = v;
	}
	std::vector<std::uint64_t> node_child_count(m_nodes.size(), 0);
	std::deque<cacl_pointer_node*> height_queue;
	// replace import/input leaf nodes with mapped node pointer
	for (auto& one_node : flat_nodes_info.flat_nodes)
	{
		auto& cur_node = m_nodes[one_node.idx];
		auto cur_child_size = one_node.children.size();
		if (cur_child_size)
		{
			node_child_count[one_node.idx] = cur_child_size;
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
				cur_node.add_child(&m_nodes[m_name_to_idx[cur_child_name]]);
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
			one_parent->m_height = std::max(one_parent->m_height, cur_node->m_height + 1);

			auto cur_child_count = node_child_count[one_parent->m_node_idx]--;
			if (cur_child_count == 1)
			{
				height_queue.push_back(one_parent);
			}
		}
	}

}


void formula_value_tree::pretty_print() const
{
	std::unordered_set<std::string> reached_names;
	const auto& temp_nodes = m_node_tree.nodes();
	for (auto[k, v] : m_node_tree.name_to_idx())
	{
		const auto& cur_node = temp_nodes[v];
		if (cur_node.cacl_type == node_type::root)
		{
			if (reached_names.count(k) == 0)
			{
				auto pre_result = cur_node.pretty_print(m_node_values, reached_names);
				if (pre_result != k)
				{
					std::cout << k << "=" << pre_result << std::endl;

				}
			}
			
		}
	}
}


void formula_value_tree::pretty_print_value() const
{
	std::unordered_set<std::string> reached_names;
	const auto& temp_nodes = m_node_tree.nodes();
	for (auto[k, v] : m_node_tree.name_to_idx())
	{
		const auto& cur_node = temp_nodes[v];
		if (cur_node.cacl_type == node_type::root)
		{
			if (reached_names.count(k) == 0)
			{
				auto pre_result = cur_node.pretty_print_value(m_node_values, reached_names);
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
formula_value_tree* formula_tree_mgr::load_formula_group(const std::string& formula_group_name, const formula_desc& output_node)
{
	auto cur_iter = named_formulas.find(formula_group_name);
	if (cur_iter != named_formulas.end())
	{
		return new formula_value_tree(*cur_iter->second);
	}
	else
	{
		auto cur_flat_info = formula_desc_flat(output_node);
		auto cur_tree = new formula_structure_tree(cur_flat_info);
		named_formulas[formula_group_name] = cur_tree;
		return new formula_value_tree(*cur_tree);
	}
}