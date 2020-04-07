#include <cacl_pointer_node.h>
#include <formula_pointer_tree.h>
#include <iostream>

using namespace spiritsaway::formula_tree::runtime;

cacl_pointer_node::cacl_pointer_node(formula_pointer_tree* in_tree, const std::string& output_name, node_type operation)
	: cacl_node_base(output_name, operation)
	, tree(in_tree)
{

}
cacl_pointer_node::cacl_pointer_node()
	:cacl_node_base()
{

}
//cacl_pointer_node::cacl_pointer_node(const cacl_pointer_node& other)
//	: cacl_node_base(other)
//	, height(other.height)
//	, parents(other.parents)
//	, children(other.children)
//	, tree(other.tree)
//{
//
//}
//cacl_pointer_node& cacl_pointer_node::operator=(const cacl_pointer_node& other)
//{
//	if (this == &other)
//	{
//		return *this;
//	}
//
//}
void cacl_pointer_node::add_child(cacl_pointer_node* child)
{
	children.push_back(child);
	child->parents.push_back(this);
}
void cacl_pointer_node::update_value(double new_value)
{
	if (new_value == value)
	{
		return;
	}
	value = new_value;
	for (auto one_parent : parents)
	{
		tree->add_node_to_update_queue(one_parent);
	}
}
bool cacl_pointer_node::update()
{
	std::vector<double> child_values(children.size(), 1.0);
	for(std::uint32_t i = 0; i< children.size(); i++)
	{
		child_values[i] = children[i]->value;
	}
	return cacl_node_base::update(child_values);
}

std::string cacl_pointer_node::pretty_print(std::unordered_set<std::string>& print_names) const
{
	switch (cacl_type)
	{
	case node_type::literal:
		return std::to_string(value);
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
	for (auto one_child : children)
	{
		child_formulas.push_back(one_child->pretty_print(print_names));
	}
	
	auto result = cacl_node_base::print_formula(child_formulas);
	if (result.size() >= 50)
	{
		std::cout << name << " = " << result << std::endl;
		return name;
	}
	else
	{
		if (cacl_type == node_type::root)
		{
			std::cout << name << " = " << result << std::endl;
			return name;
		}
		return result;
	}
}
std::string cacl_pointer_node::pretty_print_value(std::unordered_set<std::string>& print_names) const
{
	if (print_names.count(name) != 0)
	{
		return "";
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
	for (auto one_child : children)
	{
		child_formulas.push_back(one_child->pretty_print_value(print_names));
	}
	
	auto result = cacl_node_base::print_formula(child_formulas);
	if (result.size() >= 50)
	{
		std::cout << name + "(" + std::to_string(value) + ")" << " = " << result << std::endl;
		return name + "(" + std::to_string(value) + ")";;
	}
	else
	{
		if (cacl_type == node_type::root)
		{
			std::cout << name + "(" + std::to_string(value) + ")" << " = " << result << std::endl;
			return name + "(" + std::to_string(value) + ")";;
		}
		return result;
	}
}


