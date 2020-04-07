#pragma once

#include "formula_desc.h"
#include "cacl_node_base.h"
namespace spiritsaway::formula_tree::runtime
{
	class formula_pointer_tree;
	class cacl_pointer_node: public cacl_node_base
	{
		std::vector<cacl_pointer_node*> children;
		std::vector<cacl_pointer_node*> parents;
		formula_pointer_tree* tree;
		friend class formula_pointer_tree;
	public:
		cacl_pointer_node();
		cacl_pointer_node(formula_pointer_tree* tree, const std::string& output_name, node_type cacl_type);
		//cacl_pointer_node(const cacl_pointer_node& other) = default;
		//cacl_pointer_node& operator=(const cacl_pointer_node& other) = default;
		void add_child(cacl_pointer_node* child);
		void update_value(double new_value);
		bool update();
		std::string pretty_print(std::unordered_set<std::string>& print_names) const;
		std::string pretty_print_value(std::unordered_set<std::string>& print_names) const;
	};
}
