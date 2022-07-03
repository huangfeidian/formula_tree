#pragma once
#include <deque>
#include <queue>
#include <optional>
#include "cacl_pointer_node.h"

namespace spiritsaway::formula_tree::runtime
{
	class formula_tree_mgr;
	class formula_value_tree;

	struct node_compare
	{
		bool operator()(const cacl_pointer_node* a, const cacl_pointer_node* b) const
		{
			return a->height() > b->height();
		}
	};
	class formula_structure_tree
	{
		std::unordered_map<std::string, std::uint32_t> m_name_to_idx;
		std::vector<cacl_pointer_node> m_nodes;
	public:
		formula_structure_tree(const formula_desc_flat& flat_nodes_info);
		const std::unordered_map<std::string, std::uint32_t>& name_to_idx() const
		{
			return m_name_to_idx;
		}
		const std::vector<cacl_pointer_node>& nodes() const
		{
			return m_nodes;
		}

		friend class formula_tree_mgr;
	};
	using attr_value_pair = std::pair<std::string, double>;
	class formula_value_tree
	{
		std::vector<double> m_node_values;
		const formula_structure_tree& m_node_tree;

		std::priority_queue<const cacl_pointer_node*, std::vector<const cacl_pointer_node*>, node_compare> update_queue;
		std::vector<std::uint8_t> m_node_in_queue_flag;
		std::vector<std::uint32_t> m_in_queue_nodes;
		
		bool m_debug_on = false;
		bool add_node_to_update_queue(cacl_pointer_node* new_node);
		friend class cacl_pointer_node;
		std::vector<attr_value_pair> process_update_queue();

		friend class formula_tree_mgr;
	public:
		formula_value_tree(const formula_structure_tree& in_node_tree);
		std::optional<double> get_attr_value(const std::string& attr_name) const;
		std::vector<attr_value_pair> update_attr(const std::string& attr_name, double value);
		std::vector<attr_value_pair> update_attr_batch(const std::vector<attr_value_pair>& input_attrs);
		const std::unordered_map<std::string, std::uint32_t>& name_to_node_idx() const
		{
			return m_node_tree.name_to_idx();
		}
		~formula_value_tree();
		void pretty_print() const;
		void pretty_print_value() const;
		void set_debug(bool debug_on);

	};
	class formula_tree_mgr
	{
		std::unordered_map<std::string, formula_structure_tree*> named_formulas;
		formula_tree_mgr();
	public:
		formula_value_tree* load_formula_group(const std::string& formula_group_name, const formula_desc& output_node);
		static formula_tree_mgr& instance();


	};
}