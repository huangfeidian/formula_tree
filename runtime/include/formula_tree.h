#pragma once
#include <deque>
#include <queue>
#include <optional>
#include "calc_node.h"

namespace spiritsaway::formula_tree::runtime
{
	class formula_tree_mgr;
	class formula_value_tree;
	struct formula_desc_flat
	{
		std::vector<cacl_node_desc> flat_nodes; // 相关tree里的所有节点的索引
		std::unordered_map<std::string, std::uint32_t> node_indexes; // 所有有名字的节点的索引
		formula_desc_flat(const formula_desc& input_trees);
	};
	struct node_compare
	{
		bool operator()(const calc_node* a, const calc_node* b) const
		{
			return a->height() > b->height();
		}
	};
	class formula_structure_tree
	{
		std::unordered_map<std::string, std::uint32_t> m_name_to_idx;
		std::vector<calc_node> m_nodes;
		std::vector<double> m_literals;
	public:
		formula_structure_tree(const formula_desc_flat& flat_nodes_info);
		const std::unordered_map<std::string, std::uint32_t>& name_to_idx() const
		{
			return m_name_to_idx;
		}
		const std::vector<calc_node>& nodes() const
		{
			return m_nodes;
		}
		const std::vector<double> literals() const
		{
			return m_literals;
		}
		friend class formula_tree_mgr;
	};
	struct attr_update_info
	{
		std::uint32_t node_idx;
		std::uint32_t watch_idx;
		double value;
	};
	class formula_value_tree
	{
		std::vector<double> m_node_values;
		std::vector< attr_update_info> m_updated_attrs;
		const formula_structure_tree& m_node_tree;

		std::priority_queue<const calc_node*, std::vector<const calc_node*>, node_compare> update_queue;
		std::vector<std::uint8_t> m_node_in_queue_flag;
		std::vector<std::uint32_t> m_node_watch_idxes;
		std::vector<std::uint32_t> m_in_queue_nodes;

		std::function<void(const std::string&)> m_debug_print_func;
		bool add_node_to_update_queue(const calc_node* new_node);
		friend class calc_node;
		void process_update_queue();
		void update_in_constructor();
		friend class formula_tree_mgr;
	public:
		formula_value_tree(const formula_structure_tree& in_node_tree);
		std::optional<double> get_attr_value(const std::string& attr_name) const;
		std::optional<double> get_attr_value(std::uint32_t node_idx) const;
		void update_attr(const std::string& attr_name, double value);
		void update_attr(const std::uint32_t node_idx, double value);
		void update_attr_batch(const std::vector<std::pair<std::string, double>>& input_attrs);
		void update_attr_batch(const std::vector<std::pair<std::uint32_t, double>>& input_attrs);
		std::uint32_t name_to_node_idx(const std::string& attr_name) const;
		const std::string& node_idx_to_name(std::uint32_t node_idx) const
		{
			static std::string invalid_name = "invalid";
			if (node_idx >= m_node_tree.nodes().size())
			{
				return invalid_name;
			}
			return m_node_tree.nodes()[node_idx].name;
		}
		~formula_value_tree();
		std::string pretty_print() const;
		std::string pretty_print_value() const;
		void set_debug(std::function<void(const std::string&)> debug_func);
		const std::vector< attr_update_info>& updated_attrs() const
		{
			return m_updated_attrs;
		}
		// 将一些attr的名字映射为外部的一些索引 更新attr的时候顺便会更新m_updated_attrs 外部可以通过这些索引来加速处理 不再需要名字来查找
		void watch_nodes(const std::unordered_map<std::string, std::uint32_t>& watch_indexes);

	};
	class formula_tree_mgr
	{
		std::unordered_map<std::string, cacl_tree> total_trees;
		std::string repo_dir;
		std::unordered_map<std::string, std::unique_ptr<formula_structure_tree>> named_formulas;
		formula_tree_mgr();
	public:
		formula_value_tree* load_formula_group(const std::string& formula_group_name, const formula_desc& output_node);
		static formula_tree_mgr& instance();
		const cacl_tree& load_tree(const std::string& tree_name);
		void set_repo_dir(const std::string& repo_dir);


	};
}