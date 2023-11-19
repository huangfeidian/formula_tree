#include "formula_tree.h"
#include <iostream>
using namespace spiritsaway::formula_tree::runtime;

int main()
{
	std::unordered_set<std::string> related_outputs =
	{
		"armor",
		"dexterity",
		"intelligence",
		"magic_atk",
		"magic_resist",
		"max_hp",
		"output_magic_atk",
		"output_phy_atk",
		"phy_atk",
		"strength",
	};
	formula_tree_mgr::instance().set_repo_dir("../../data/export/");
	auto cur_formula_tree = formula_tree_mgr::instance().load_formula_group("player", formula_desc{ related_outputs });
	std::cout << "formula print begin >>>>>>>>>>>>" << std::endl;
	std::cout<<cur_formula_tree->pretty_print();
	std::cout << "formula print end <<<<<<<<<<<<<<" << std::endl;

	std::cout << "formula print value begin >>>>>>>>>>>>" << std::endl;
	std::cout << cur_formula_tree->pretty_print_value();
	std::cout << "formula print value end <<<<<<<<<<<<<<" << std::endl;
}