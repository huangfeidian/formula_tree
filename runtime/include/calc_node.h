#pragma once

#include "formula_desc.h"
namespace spiritsaway::formula_tree::runtime
{
	class formula_structure_tree;
	class formula_value_tree;
	class calc_node
	{
		std::vector<calc_node*> children;
		std::vector<std::uint32_t> m_children_idxes;
		std::vector<calc_node*> parents;
		formula_structure_tree* tree = nullptr;
		std::uint64_t m_node_idx = 0;
		node_type cacl_type;
		std::uint32_t m_height = 0;
		std::string name;
		friend class formula_structure_tree;
		friend class formula_value_tree;
	public:
		calc_node();
		calc_node(formula_structure_tree* tree, std::uint64_t in_node_idx, const std::string& output_name, node_type cacl_type);

		void add_child(calc_node* child);
		void update_value(formula_value_tree* value_tree, std::vector<double>& node_values, double new_value) const;
		bool update(std::vector<double>& node_values) const;
		double uniform(double a, double b) const;
		std::uint64_t node_idx() const
		{
			return m_node_idx;
		}
		std::uint32_t height() const
		{
			return m_height;
		}
		std::string pretty_print(const std::vector<double>& node_values, std::unordered_set<std::string>& print_names) const;
		std::string pretty_print_value(const std::vector<double>& node_values, std::unordered_set<std::string>& print_names) const;
		std::string print_formula(const std::vector<double>& node_values, const std::vector<std::string>& arg_names) const;
	};
}
