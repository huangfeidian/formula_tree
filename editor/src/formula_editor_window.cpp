#include <fstream>
#include <streambuf>

#include <qfiledialog.h>
#include <tree_editor/common/dialogs/path_config_dialog.h>
#include <tree_editor/common/choice_manager.h>
#include <tree_editor/common/graph/tree_instance.h>

#include <any_container/decode.h>

#include "formula_editor_window.h"
#include "formula_nodes.h"

using namespace spiritsaway::tree_editor;
using namespace spiritsaway::formula_tree::editor;
using namespace std;
std::string formula_editor_window::new_file_name()
{
	std::string temp = fmt::format("new_formula_tree_{}.json", get_seq());
	std::filesystem::path temp_path = data_folder / temp;
	while (already_open(temp) || std::filesystem::exists(temp_path))
	{
		temp = fmt::format("new_formula_tree_{}.json", get_seq());
		temp_path = data_folder / temp;
	}
	return temp;
}
bool formula_editor_window::load_config()
{

	auto config_file_name = "formula_node_config.json";
	std::string notify_info;
	std::string node_desc_path;
	std::string choice_desc_path;
	std::string save_path;
	if (!std::filesystem::exists(std::filesystem::path(config_file_name)))
	{
		path_req_desc node_req;
		node_req.name = "formula node types";
		node_req.tips = "file to provide all formula nodes";
		node_req.extension = ".json";

		path_req_desc choice_req;
		choice_req.name = "formula named attr";
		choice_req.tips = "file to provide named attr";
		choice_req.extension = ".json";

		path_req_desc save_path_req;
		save_path_req.name = "data save dir";
		save_path_req.tips = "directory to save data files";
		save_path_req.extension = "";

		std::vector<path_req_desc> path_reqs;
		path_reqs.push_back(node_req);
		path_reqs.push_back(choice_req);
		path_reqs.push_back(save_path_req);
		
		auto cur_dialog = new path_config_dialog(path_reqs, config_file_name, this);
		auto temp_result = cur_dialog->run();
		if (!cur_dialog->valid)
		{
			QMessageBox::about(this, QString("Error"),
				QString::fromStdString("invalid formula node config"));
			return false;
		}
		node_desc_path = temp_result[0];
		choice_desc_path = temp_result[1];
		save_path = temp_result[2];
	}
	else
	{
		auto config_json_variant = load_json_file(config_file_name);
		if (std::holds_alternative<std::string>(config_json_variant))
		{
			auto notify_info = "config file: " + std::get<std::string>(config_json_variant);
			QMessageBox::about(this, QString("Error"),
				QString::fromStdString(notify_info));
			return false;
		}
		auto json_content = std::get<json::object_t>(config_json_variant);

		std::vector<std::string> temp_result;
		std::vector<std::string> path_keys = { "formula node types", "formula named attr" , "data save dir" };
		for (auto one_key : path_keys)
		{
			auto cur_value_iter = json_content.find(one_key);
			if (cur_value_iter == json_content.end())
			{
				notify_info = "config content should be should has key " + one_key;
				QMessageBox::about(this, QString("Error"),
					QString::fromStdString(notify_info));
				return false;
			}
			if (!cur_value_iter->second.is_string())
			{
				notify_info = "config content should for key " + one_key + " should be str";
				QMessageBox::about(this, QString("Error"),
					QString::fromStdString(notify_info));
				return false;
			}
			temp_result.push_back(cur_value_iter->second.get<std::string>());
		}
		node_desc_path = temp_result[0];
		choice_desc_path = temp_result[1];
		save_path = temp_result[2];
	}
	// choice first 
	auto choice_json_variant = load_json_file(choice_desc_path);
	if (std::holds_alternative<std::string>(choice_json_variant))
	{
		auto notify_info = "chocie file: " + std::get<std::string>(choice_json_variant);
		QMessageBox::about(this, QString("Error"),
			QString::fromStdString(notify_info));
		return false;
	}
	else
	{
		choice_manager::instance().load_from_json(std::get<json::object_t>(choice_json_variant));
	}
	
	//nodes depends on choice
	auto node_json_variant = load_json_file(node_desc_path);
	if (std::holds_alternative<std::string>(node_json_variant))
	{
		auto notify_info = "nodes file: " + std::get<std::string>(node_json_variant);
		QMessageBox::about(this, QString("Error"),
			QString::fromStdString(notify_info));
		return false;
	}
	else
	{
		node_config_repo::instance().load_config(std::get<json::object_t>(node_json_variant));
	}

	data_folder = save_path;
	return true;
}

basic_node* formula_editor_window::create_node_from_desc(const basic_node_desc& cur_desc, basic_node* parent)
{
	auto cur_config = node_config_repo::instance().get_config(cur_desc.type);
	if (!cur_config)
	{
		return nullptr;
	}
	auto cur_node = new formula_node(cur_config.value(), dynamic_cast<formula_node*>(parent), cur_desc.idx);
	if (parent)
	{
		parent->add_child(cur_node);
	}
	cur_node->color = cur_desc.color;
	cur_node->_is_collapsed = cur_desc.is_collpased;
	cur_node->comment = cur_desc.comment;
	cur_node->refresh_editable_items();
	cur_node->set_extra(json(cur_desc.extra));
	return cur_node;
}