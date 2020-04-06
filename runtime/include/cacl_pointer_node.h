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
		std::uint32_t height; // for update queue priority
		formula_pointer_tree* tree;
		friend class formula_pointer_tree;
	public:
		cacl_pointer_node(formula_pointer_tree* tree, const std::string& output_name, node_type cacl_type);
		void add_child(cacl_pointer_node* child);
		void update_value(double new_value);
		bool update();
	};
}
