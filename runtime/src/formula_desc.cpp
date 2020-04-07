#include "formula_desc.h"
#include <deque>
#include <nlohmann/json.hpp>
#include <any_container/decode.h>
#include <filesystem>
#include <fstream>
#include <magic_enum.hpp>

using namespace spiritsaway::formula_tree::runtime;
using namespace spiritsaway::serialize;
bool cacl_node_desc::decode(const json& data)
{
	if (!data.is_object())
	{
		return false;
	}

	if (!serialize::decode(data["idx"], idx))
	{
		return false;
	}


	if (!serialize::decode(data["children"], children))
	{
		return false;
	}
	std::string temp_type;
	if (!serialize::decode(data["type"], temp_type))
	{
		return false;
	}
	auto opt_type = magic_enum::enum_cast<node_type>(temp_type);
	if (!opt_type)
	{
		return false;
	}
	type = opt_type.value();

	
	if (type == node_type::literal)
	{
		auto temp_value = data["extra"]["value"];
		if (!serialize::decode(temp_value, value))
		{
			return false;
		}
	}
	else if (type == node_type::import || type == node_type::input)
	{
		auto temp_value = data["extra"]["value"];
		if (!serialize::decode(temp_value, name))
		{
			return false;
		}
	}
	return true;


}
void cacl_tree_repo::set_repo_dir(const std::string& in_repo_dir)
{
	repo_dir = in_repo_dir;
	for (const auto& one_entry : std::filesystem::recursive_directory_iterator(in_repo_dir))
	{
		if (one_entry.is_directory())
		{
			continue;
		}
		std::ifstream tree_file(one_entry);
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
cacl_tree_repo& cacl_tree_repo::instance()
{
	static cacl_tree_repo the_one;
	return the_one;
}
const cacl_tree& cacl_tree_repo::load_tree(const std::string& tree_name)
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
	auto& tree_repo = cacl_tree_repo::instance();
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
		for (auto[one_idx, one_node] : cur_tree_desc.nodes)
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
		for (auto[pre_idx, new_idx] : tree_node_index_map)
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
