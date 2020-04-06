#include <cacl_pointer_node.h>
#include <formula_pointer_tree.h>


using namespace spiritsaway::formula_tree::runtime;

cacl_pointer_node::cacl_pointer_node(formula_pointer_tree* in_tree, const std::string& output_name, node_type operation)
	: cacl_node_base(output_name, operation)
	, tree(in_tree)
{

}
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


