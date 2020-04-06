#pragma once
#include <tree_editor/common/graph/basic_node.h>

namespace spiritsaway::tree_editor
{
	class formula_node : public config_node
	{
	public:
		formula_node(const node_config& _config, formula_node* _parent, std::uint32_t _idx);
		basic_node* clone_self(basic_node* _parent) const;
		std::string display_text() const;
		json to_json() const;
		basic_node* create_node(std::string _type, basic_node* _parent, std::uint32_t _idx);
		bool set_extra(const json::object_t& data);
	};
}
