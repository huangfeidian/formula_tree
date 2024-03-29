﻿#include "calc_node.h"
#include "formula_tree.h"
#include <iostream>
#include <random>

using namespace spiritsaway::formula_tree::runtime;

calc_node::calc_node(formula_structure_tree* in_tree, std::uint32_t in_node_idx, const std::string& output_name, node_type operation)
	: name(output_name)
	, cacl_type(operation)
	, tree(in_tree)
	, m_node_idx(in_node_idx)
{

}
calc_node::calc_node()
{

}

void calc_node::add_child(calc_node* child)
{
	children.push_back(child);
	m_children_idxes.push_back(child->m_node_idx);
	child->parents.push_back(this);
}
void calc_node::update_value(formula_value_tree* value_tree, std::vector<double>& node_values, double new_value) const
{
	if (new_value == node_values[m_node_idx])
	{
		return;
	}
	node_values[m_node_idx] = new_value;
	for (auto one_parent : parents)
	{
		value_tree->add_node_to_update_queue(one_parent);
	}
}


std::string calc_node::pretty_print(const std::vector<double>& node_values, std::unordered_set<std::string>& print_names, std::ostringstream& oss) const
{
	switch (cacl_type)
	{
	case node_type::literal:
		return std::to_string(node_values[m_node_idx]);
	case node_type::input:
		return name;
	case node_type::import:
		return name;
	case node_type::root:
	{
		if (print_names.count(name) == 1)
		{
			return name;
		}
		print_names.insert(name);
		break;
	}
	default:
		break;

	}
	std::vector<std::string> child_formulas;
	for (auto one_child: children)
	{
		child_formulas.push_back(one_child->pretty_print(node_values, print_names, oss));
	}
	
	auto result = calc_node::print_formula(node_values, child_formulas);
	if (result.size() >= 50)
	{
		oss << name << " = " << result << std::endl;
		return name;
	}
	else
	{
		if (cacl_type == node_type::root)
		{
			oss << name << " = " << result << std::endl;
			return name;
		}
		return result;
	}
}
std::string calc_node::pretty_print_value(const std::vector<double>& node_values, std::unordered_set<std::string>& print_names, std::ostringstream& oss) const
{
	auto value = node_values[m_node_idx];
	if (print_names.count(name) != 0)
	{
		return name + "(" + std::to_string(value) + ")";
	}
	
	switch (cacl_type)
	{
	case node_type::literal:
		return std::to_string(value);
	case node_type::input:
		return name + "(" +std::to_string(value) + ")";
	case node_type::import:
		return name + "(" + std::to_string(value) + ")";
	case node_type::root:
	{
		if (print_names.count(name) == 1)
		{
			return name + "(" + std::to_string(value) + ")";
		}
		print_names.insert(name);
		break;
	}
	default:
		break;

	}
	std::vector<std::string> child_formulas;
	for (auto one_child: children)
	{
		child_formulas.push_back(one_child->pretty_print_value(node_values, print_names, oss));
	}
	
	auto result = calc_node::print_formula(node_values, child_formulas);
	if (result.size() >= 50)
	{
		oss << name + "(" + std::to_string(value) + ")" << " = " << result << std::endl;
		return name + "(" + std::to_string(value) + ")";;
	}
	else
	{
		if (cacl_type == node_type::root)
		{
			oss << name + "(" + std::to_string(value) + ")" << " = " << result << std::endl;
			return name + "(" + std::to_string(value) + ")";
		}
		return result;
	}
}

double calc_node::uniform(double a, double b) const
{
	static std::random_device rd;
	static std::mt19937 seed(rd());
	static std::uniform_real_distribution<double> cur_dis(0, 1);
	if (a == b)
	{
		return a;
	}
	if (a > b)
	{
		std::swap(a, b);
	}
	return a + cur_dis(seed)* (b - a);
}
bool calc_node::update(std::vector<double>& node_values) const
{
	double result;
	switch (cacl_type)
	{
	case node_type::root:
		result = node_values[m_children_idxes[0]];
		break;
	case node_type::literal:
		result = node_values[m_node_idx];
		break;
	case node_type::add:
		result = 0.0;
		for (auto one_child: m_children_idxes)
		{
			result += node_values[one_child];
		}
		break;
	case node_type::dec:
		result = node_values[m_children_idxes[0]] - node_values[m_children_idxes[1]];
		break;
	case node_type::mul:
		result = 1.0;
		for (auto one_child: m_children_idxes)
		{
			result *= node_values[one_child];
		}
		break;
	case node_type::div:
		if (node_values[m_children_idxes[1]] == 0)
		{
			result = node_values[m_children_idxes[0]];
		}
		else
		{
			result = node_values[m_children_idxes[0]] / node_values[m_children_idxes[1]];
		}
		break;
	case node_type::pow:
		result = std::pow(node_values[m_children_idxes[0]], node_values[m_children_idxes[1]]);
		break;
	case node_type::max:
		result = node_values[m_children_idxes[0]];
		for (auto one_child: m_children_idxes)
		{
			if (node_values[one_child] > result)
			{
				result = node_values[one_child];
			}
		}
		break;
	case node_type::min:
		result = node_values[m_children_idxes[0]];
		for (auto one_child: m_children_idxes)
		{
			if (node_values[one_child] < result)
			{
				result = node_values[one_child];
			}
		}
		break;
	case node_type::average:
		result = 0.0;
		for (auto one_child: m_children_idxes)
		{
			result += node_values[one_child];
		}
		result /= children.size();
		break;
	case node_type::percent_add:
		result = (1 + node_values[m_children_idxes[0]] / 100.0) *node_values[m_children_idxes[1]];
		break;
	case node_type::clamp:
	{
		double a = node_values[m_children_idxes[0]];
		double b = node_values[m_children_idxes[1]];
		double c = node_values[m_children_idxes[2]];
		if (a < b)
		{
			result = b;
		}
		else if (a > c)
		{
			result = c;
		}
		else
		{
			result = a;
		}
		break;
	}
		
	default:
		result = node_values[m_node_idx];
		break;
	}
	if (result == node_values[m_node_idx])
	{
		return false;
	}
	else
	{
		node_values[m_node_idx] = result;
		return true;
	}
}

std::string calc_node::print_formula(const std::vector<double>& node_values, const std::vector<std::string>& arg_names) const
{
	auto value = node_values[m_node_idx];
	switch (cacl_type)
	{
	case node_type::root:
		return  arg_names[0];
		break;
	case node_type::literal:
		return  std::to_string(value);
	case node_type::import:
		return  std::to_string(value);
	case node_type::input:
		return  std::to_string(value);
	case node_type::add:
		return  "(" + arg_names[0] + "+" + arg_names[1] + ")";
	case node_type::dec:
		return  "(" + arg_names[0] + "-" + arg_names[1] + ")";
	case node_type::mul:
		return  arg_names[0] + "*" + arg_names[1];
	case node_type::div:
		return  arg_names[0] + "/" + arg_names[1];
	case node_type::pow:
		return  "pow(" + arg_names[0] + "," + arg_names[1] + ")";
	case node_type::max:
	{
		std::string result = "max(";
		for(auto one_arg: arg_names)
		{
			result+=one_arg + ",";
		}
		result += ")";
		return result;
	}
	case node_type::min:
	{
		std::string result = "min(";
		for(auto one_arg: arg_names)
		{
			result+=one_arg + ",";
		}
		result += ")";
		return result;
	}
	case node_type::average:
	{
		std::string result = "average(";
		for(auto one_arg: arg_names)
		{
			result+=one_arg + ",";
		}
		result += ")";
		return result;
	}
	case node_type::clamp:
		return  "clamp(" + arg_names[0] + "," + arg_names[1] + ","+ arg_names[2] + ")";
	case node_type::percent_add:
		return  "(1 +" + arg_names[0] + "/100)*" + arg_names[1];
	default:
		return "";
	}
}


