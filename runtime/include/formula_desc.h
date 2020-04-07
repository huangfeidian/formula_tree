#pragma once
#include <string>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <nlohmann/json.hpp>
#include "node_type.h"
namespace spiritsaway::formula_tree::runtime
{
	using json = nlohmann::json;
	struct cacl_node_desc
	{
		std::string name; //name for import / output / input nodes
		double value;//   double for literal nodes
		std::uint32_t idx;
		// for non leaf nodes
		node_type type;
		std::vector<std::uint32_t> children;
		bool decode(const json& data);
	};
	// every formula tree has a output name
	struct cacl_tree
	{
		std::unordered_map<std::uint32_t, cacl_node_desc> nodes;
	};
	class cacl_tree_repo
	{
		std::unordered_map<std::string, cacl_tree> total_trees;
		std::string repo_dir;
	public:
		const cacl_tree& load_tree(const std::string& tree_name);

		static cacl_tree_repo& instance();
		void set_repo_dir(const std::string& repo_dir);
	};
	struct formula_desc
	{
		std::unordered_set<std::string> output_names;
	};
	struct formula_desc_flat
	{
		std::vector<cacl_node_desc> flat_nodes;
		std::unordered_map<std::string, std::uint32_t> node_indexes;
		std::vector<std::uint32_t> heights;
		formula_desc_flat(const formula_desc& input_trees);
	};
}