#include <tree_editor/common/dialogs/editable_item.h>
#include <tree_editor/common/choice_manager.h>
#include "formula_nodes.h"

using namespace spiritsaway::tree_editor;
using namespace std;

json formula_node::to_json() const
{
	auto result = config_node::to_json();
	json::object_t extra;
	if (_type == "literal")
	{
		extra["value"] = _show_widget->find("value")->to_json()["value"];
	}
	else if(_type == "import" || _type == "input")
	{
		extra["value"] = _show_widget->find("value")->to_json()["value"];
	}
	result["extra"] = extra;
	return result;
}
std::string formula_node::display_text() const
{
	if (_type == "literal")
	{
		return std::to_string(_idx) + ":" + _type + ":" + std::to_string(_show_widget->find("value")->_value.get<double>());
	}
	else if (_type == "import")
	{
		auto cur_str = choice_manager::instance().get_choice_comment("import_attrs", _show_widget->find("value")->_value.get<std::string>());
		cur_str = cur_str.substr(cur_str.find(":"));
		return std::to_string(_idx) + ":" + _type  + cur_str;

	}
	else if (_type == "input")
	{
		auto cur_str = choice_manager::instance().get_choice_comment("input_attrs", _show_widget->find("value")->_value.get<std::string>());
		cur_str = cur_str.substr(cur_str.find(":"));
		return std::to_string(_idx) + ":" + _type  + cur_str;
	}
	else
	{
		return basic_node::display_text();
	}
	
}
basic_node* formula_node::clone_self(basic_node* _parent) const
{
	auto new_node = new formula_node(_config, reinterpret_cast<formula_node*>(_parent), 0);
	new_node->_show_widget = std::dynamic_pointer_cast<struct_items>(_show_widget->clone());
	return new_node;
}
formula_node::formula_node(const node_config& _config, formula_node* _parent, std::uint32_t _idx)
	: config_node(_config, _parent, _idx)
{

}
basic_node* formula_node::create_node(std::string _type, basic_node* _parent, std::uint32_t _idx)
{
	auto cur_config = node_config_repo::instance().get_config(_type);
	if (!cur_config)
	{
		return nullptr;
	}
	return new formula_node(cur_config.value(), reinterpret_cast<formula_node*>(_parent), _idx);
}
bool formula_node::set_extra(const json::object_t& data)
{
	auto value_iter = data.find("value");
	if (value_iter == data.end())
	{
		return false;
	}
	if (_type == "literal")
	{
		if (!value_iter->second.is_number_float())
		{
			return false;
		}
		auto cur_widget = _show_widget->find("value");
		if (!cur_widget)
		{
			return false;
		}
		return cur_widget->assign(value_iter->second);
	}
	else if (_type == "import" || _type == "input")
	{
		if (!value_iter->second.is_string())
		{
			return false;
		}
		auto cur_widget = _show_widget->find("value");
		if (!cur_widget)
		{
			return false;
		}
		return cur_widget->assign(value_iter->second);
	}
	return true;
}