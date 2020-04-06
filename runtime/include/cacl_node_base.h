#pragma once
#include <random>
#include "formula_desc.h"

namespace spiritsaway::formula_tree::runtime
{
	struct cacl_node_base
	{
		double value;
		const node_type cacl_type;
		std::uint32_t height;
		const std::string name;
		// return true if result is different than pre value
		// else return false
		bool update(const std::vector<double>& children_values);
		double uniform(double a, double b);
		cacl_node_base(const std::string& output_name, node_type cacl_type);
		std::string print_formula(const std::string& my_name, const std::vector<std::string>& arg_names) const;
		std::string print_formula_value(const std::string& my_name, const std::vector<std::string>& arg_names) const;
	};
	struct node_compare
	{
		bool operator()(const cacl_node_base* a, const cacl_node_base* b) const
		{
			return a->height < b->height;
		}
	};
}