#pragma once
#include <vector>
#include <optional>

#include "formula_desc.h"

namespace spiritsaway::formula_tree::runtime
{
	using attr_value_pair = std::pair<std::string, double>;
	class formula_tree_interface
	{
	public:
		formula_tree_interface(const formula_desc_flat& flat_nodes_info, const std::vector<attr_value_pair>& init_values)
		{
			
		}
		virtual std::optional<double> get_attr_value(const std::string& attr_name) = 0;
		virtual std::vector<attr_value_pair> update_attr(const std::string& attr_name, double value) = 0;
		virtual std::vector<attr_value_pair> update_attr_batch(const std::vector<attr_value_pair>& input_attrs) = 0;

		virtual ~formula_tree_interface();
	};
}
