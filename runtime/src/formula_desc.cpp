#include "formula_desc.h"
#include <deque>
#include <nlohmann/json.hpp>
#include <any_container/decode.h>
#include <filesystem>
#include <fstream>
#include <magic_enum/magic_enum.hpp>

using namespace spiritsaway::formula_tree::runtime;
using namespace spiritsaway::serialize;
bool cacl_node_desc::decode(const json& data)
{
	if (!data.is_object())
	{
		return false;
	}

	if (!serialize::decode(data["idx"], idx))
	{
		return false;
	}


	if (!serialize::decode(data["children"], children))
	{
		return false;
	}
	std::string temp_type;
	if (!serialize::decode(data["type"], temp_type))
	{
		return false;
	}
	auto opt_type = magic_enum::enum_cast<node_type>(temp_type);
	if (!opt_type)
	{
		return false;
	}
	type = opt_type.value();

	
	if (type == node_type::literal)
	{
		auto temp_value = data["extra"]["value"];
		if (!serialize::decode(temp_value, value))
		{
			return false;
		}
	}
	else if (type == node_type::import || type == node_type::input)
	{
		auto temp_value = data["extra"]["value"];
		if (!serialize::decode(temp_value, name))
		{
			return false;
		}
	}
	return true;


}

json cacl_node_desc::encode() const
{
	json result;
	if (type == node_type::import || type == node_type::input)
	{
		result["extra"]["value"] = name;
		
	}
	else
	{
		result["extra"]["value"] = value;

	}
	result["type"] = magic_enum::enum_name(type);
	result["children"] = children;
	result["idx"] = idx;
	return result;

}



