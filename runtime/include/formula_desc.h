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
		json encode() const;
	};
	// every formula tree has a output name
	struct cacl_tree
	{
		std::unordered_map<std::uint32_t, cacl_node_desc> nodes;
	};

	struct formula_desc
	{
		std::unordered_set<std::string> output_names;
	};

}