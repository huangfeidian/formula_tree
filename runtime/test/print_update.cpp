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
	cur_formula_tree->set_debug(true);
	cur_formula_tree->update_attr("level", 2);
}