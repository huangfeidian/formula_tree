#include "formula_tree.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <any_container/decode.h>
#include <filesystem>
#include <fstream>
#include <magic_enum.hpp>
using namespace spiritsaway::formula_tree::runtime;

formula_value_tree::formula_value_tree(const formula_structure_tree& in_node_tree)
	: m_node_tree(in_node_tree)
	, m_node_values(in_node_tree.nodes().size(), 1.0)
	, m_node_in_queue_flag(in_node_tree.nodes().size(), 0)
{

}
void formula_value_tree::process_update_queue()
{
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
			m_updated_attrs.push_back(attr_update_info{ cur_top->m_node_idx, m_node_values[cur_top->m_node_idx] });
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
std::optional<double> formula_value_tree::get_attr_value(std::uint32_t node_idx) const
{
	if (node_idx >= m_node_values.size())
	{
		return {};
	}
	else
	{
		return m_node_values[node_idx];
	}
}

void formula_value_tree::update_attr(const std::string& name, double value)
{
	std::vector<std::pair<std::string, double>> args;
	args.emplace_back(name, value);
	update_attr_batch(args);
}
void formula_value_tree::update_attr(const std::uint32_t node_idx, double value)
{
	if (node_idx >= m_node_values.size())
	{
		return;
	}

	m_node_tree.nodes()[node_idx].update_value(this, m_node_values, value);
	m_updated_attrs.clear();
	return process_update_queue();
}
void formula_value_tree::update_attr_batch(const std::vector<std::pair<std::string, double>>& input_attrs)
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
	m_updated_attrs.clear();
	return process_update_queue();
}

void formula_value_tree::update_attr_batch(const std::vector<std::pair<std::uint32_t, double>>& input_attrs)
{
	for (auto one_attr : input_attrs)
	{
		if (one_attr.first >= m_node_values.size())
		{
			continue;
		}
		m_node_tree.nodes()[one_attr.first].update_value(this, m_node_values, one_attr.second);
	}
	m_updated_attrs.clear();
	return process_update_queue();
}

bool formula_value_tree::add_node_to_update_queue(const calc_node* new_node)
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
		auto cur_pointer_node = calc_node(this, m_nodes.size(), cur_node_name, one_node.type);
		m_nodes.push_back(cur_pointer_node);
	}
	// map names to node pointer
	auto node_begin_pointer = m_nodes.data();
	for (const auto& [k, v] : flat_nodes_info.node_indexes)
	{
		m_name_to_idx[k] = v;
	}
	std::vector<std::uint64_t> node_child_count(m_nodes.size(), 0);
	std::deque<calc_node*> height_queue;
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

void formula_tree_mgr::set_repo_dir(const std::string& in_repo_dir)
{
	repo_dir = in_repo_dir;
	for (const auto& one_entry : std::filesystem::recursive_directory_iterator(in_repo_dir))
	{
		if (one_entry.is_directory())
		{
			continue;
		}
		std::ifstream tree_file(one_entry.path().string());
		std::string tree_file_content((std::istreambuf_iterator<char>(tree_file)), std::istreambuf_iterator<char>());
		if (!json::accept(tree_file_content))
		{
			continue;

		}
		auto tree_json_content = json::parse(tree_file_content);
		if (!tree_json_content.is_object())
		{
			continue;

		}
		auto node_info_iter = tree_json_content.find("nodes");
		if (node_info_iter == tree_json_content.end())
		{
			continue;

		}
		std::vector<cacl_node_desc> nodes_info;
		if (!spiritsaway::serialize::decode(*node_info_iter, nodes_info))
		{
			continue;

		}
		std::unordered_map<std::uint32_t, cacl_node_desc> map_nodes_info;
		for (const auto& one_node : nodes_info)
		{
			map_nodes_info[one_node.idx] = one_node;
		}
		auto cur_tree_name = one_entry.path().filename().stem().string();
		map_nodes_info[0].name = cur_tree_name;
		total_trees[cur_tree_name] = cacl_tree{ map_nodes_info };
	}
}

const cacl_tree& formula_tree_mgr::load_tree(const std::string& tree_name)
{
	static cacl_tree invalid_info;
	auto cur_iter = total_trees.find(tree_name);
	if (cur_iter == total_trees.end())
	{
		return invalid_info;
	}
	else
	{
		return cur_iter->second;
	}
}

formula_desc_flat::formula_desc_flat(const formula_desc& trees)
{
	auto& tree_repo = formula_tree_mgr::instance();
	std::deque<std::string> tree_tasks;
	for (auto one_tree : trees.output_names)
	{
		tree_tasks.push_back(one_tree);
	}
	bool invalid = false;
	while (!tree_tasks.empty())
	{
		auto cur_tree = tree_tasks.front();
		tree_tasks.pop_front();
		auto cur_iter = node_indexes.find(cur_tree);
		if (cur_iter != node_indexes.end())
		{
			// tree already created
			continue;
		}
		auto cur_tree_desc = tree_repo.load_tree(cur_tree);
		if (cur_tree_desc.nodes.empty())
		{
			// invalid tree
			invalid = true;
			break;
		}
		std::unordered_map<std::uint32_t, std::uint32_t> tree_node_index_map;
		for (auto [one_idx, one_node] : cur_tree_desc.nodes)
		{
			if (one_node.type == node_type::root)
			{
				node_indexes[cur_tree] = flat_nodes.size();
			}
			if (one_node.type == node_type::import)
			{
				tree_tasks.push_back(one_node.name);
			}
			if (one_node.type == node_type::input)
			{
				node_indexes[one_node.name] = flat_nodes.size();
			}
			tree_node_index_map[one_node.idx] = flat_nodes.size();
			flat_nodes.push_back(one_node);
		}
		for (auto [pre_idx, new_idx] : tree_node_index_map)
		{
			for (auto& one_child : flat_nodes[new_idx].children)
			{
				one_child = tree_node_index_map[one_child];
			}
			flat_nodes[new_idx].idx = new_idx;
		}
	}
	if (invalid)
	{
		flat_nodes.clear();
		node_indexes.clear();
	}
}