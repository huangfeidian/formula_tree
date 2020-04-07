#pragma once
#include <deque>
#include <queue>
#include "cacl_pointer_node.h"
#include "formula_tree_interface.h"

namespace spiritsaway::formula_tree::runtime
{
	class formula_tree_mgr;
	class formula_pointer_tree: public formula_tree_interface
	{
		std::priority_queue<cacl_pointer_node*, std::vector<cacl_pointer_node*>, node_compare> update_queue;
		std::unordered_map<std::string, cacl_pointer_node*> name_to_nodes;
		std::unordered_set<cacl_pointer_node*> nodes_in_queue;
		std::vector<cacl_pointer_node> all_nodes;
		bool debug_on = false;
		bool add_node_to_update_queue(cacl_pointer_node* new_node);
		friend class cacl_pointer_node;
		std::vector<attr_value_pair> process_update_queue();
		formula_pointer_tree(const formula_desc_flat& flat_nodes_info);
		formula_pointer_tree();
		friend class formula_tree_mgr;
	public:
		
		std::optional<double> get_attr_value(const std::string& attr_name) const;
		std::vector<attr_value_pair> update_attr(const std::string& attr_name, double value);
		std::vector<attr_value_pair> update_attr_batch(const std::vector<attr_value_pair>& input_attrs);
		~formula_pointer_tree();
		formula_pointer_tree* clone() const;
		void pretty_print() const;
		void pretty_print_value() const;
		void set_debug(bool debug_on);

	};
	class formula_tree_mgr
	{
		std::unordered_map<std::string, formula_pointer_tree*> named_formulas;
		formula_tree_mgr();
	public:
		formula_pointer_tree* load_formula_group(const std::string& formula_group_name, const formula_desc& output_node);
		static formula_tree_mgr& instance();


	};
}