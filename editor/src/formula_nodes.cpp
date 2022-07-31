#include <tree_editor/common/dialogs/editable_item.h>
#include <tree_editor/common/choice_manager.h>
#include "formula_nodes.h"

using namespace spiritsaway::tree_editor;
using namespace std;

json formula_node::to_json() const
{
	auto result = config_node::to_json();
	json::object_t extra;
	if (m_type == "literal")
	{
		extra["value"] = m_show_widget->find("value")->to_json()["value"];
	}
	else if(m_type == "import" || m_type == "input")
	{
		extra["value"] = m_show_widget->find("value")->to_json()["value"];
	}
	result["extra"] = extra;
	return result;
}
std::string formula_node::display_text() const
{
	if (m_type == "literal")
	{
		return std::to_string(m_idx) + ":" + m_type + ":" + std::to_string(m_show_widget->find("value")->m_value.get<double>());
	}
	else if (m_type == "import")
	{
		auto cur_str = choice_manager::instance().get_choice_comment("import_attrs", m_show_widget->find("value")->m_value.get<std::string>());
		cur_str = cur_str.substr(cur_str.find(":"));
		return std::to_string(m_idx) + ":" + m_type  + cur_str;

	}
	else if (m_type == "input")
	{
		auto cur_str = choice_manager::instance().get_choice_comment("input_attrs", m_show_widget->find("value")->m_value.get<std::string>());
		cur_str = cur_str.substr(cur_str.find(":"));
		return std::to_string(m_idx) + ":" + m_type  + cur_str;
	}
	else
	{
		return basic_node::display_text();
	}
	
}
basic_node* formula_node::clone_self(basic_node* _parent) const
{
	auto new_node = new formula_node(m_config, reinterpret_cast<formula_node*>(_parent), 0);
	new_node->m_show_widget = std::dynamic_pointer_cast<struct_items>(m_show_widget->clone());
	return new_node;
}
formula_node::formula_node(const node_config& _config, formula_node* _parent, std::uint32_t m_idx)
	: config_node(_config, _parent, m_idx)
{

}
basic_node* formula_node::create_node(std::string m_type, basic_node* _parent, std::uint32_t m_idx)
{
	auto cur_config = node_config_repo::instance().get_config(m_type);
	if (!cur_config)
	{
		return nullptr;
	}
	return new formula_node(cur_config.value(), reinterpret_cast<formula_node*>(_parent), m_idx);
}
bool formula_node::set_extra(const json::object_t& data)
{
	auto value_iter = data.find("value");
	if (value_iter == data.end())
	{
		return false;
	}
	if (m_type == "literal")
	{
		if (!value_iter->second.is_number_float())
		{
			return false;
		}
		auto cur_widget = m_show_widget->find("value");
		if (!cur_widget)
		{
			return false;
		}
		return cur_widget->assign(value_iter->second);
	}
	else if (m_type == "import" || m_type == "input")
	{
		if (!value_iter->second.is_string())
		{
			return false;
		}
		auto cur_widget = m_show_widget->find("value");
		if (!cur_widget)
		{
			return false;
		}
		return cur_widget->assign(value_iter->second);
	}
	return true;
}