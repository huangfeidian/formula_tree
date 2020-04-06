#include <cacl_node_base.h>
#include <cmath>
using namespace spiritsaway::formula_tree::runtime;

cacl_node_base::cacl_node_base(const std::string& in_name, node_type in_node_type)
	: cacl_type(in_node_type)
	, value(1.0)
	, name(in_name)
{

}

double cacl_node_base::uniform(double a, double b)
{
	static std::random_device rd;
	static std::mt19937 seed(rd);
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
bool cacl_node_base::update(const std::vector<double>& inputs)
{
	double result;
	switch (cacl_type)
	{
	case node_type::root:
		result = inputs[0];
		break;
	case node_type::literal:
		result = value;
		break;
	case node_type::add:
		result = 0.0;
		for (auto one_val : inputs)
		{
			result += one_val;
		}
		break;
	case node_type::dec:
		result = inputs[0] - inputs[1];
		break;
	case node_type::mul:
		result = 1.0;
		for (auto one_val : inputs)
		{
			result *= one_val;
		}
		break;
	case node_type::div:
		if (inputs[1] == 0)
		{
			result = inputs[0];
		}
		else
		{
			result = inputs[0] / inputs[1];
		}
		break;
	case node_type::random:
		result = uniform(inputs[0], inputs[1]);
		break;
	case node_type::condition:
		result = inputs[0] * inputs[1] + (1 - inputs[0]) * inputs[2];
		break;
	case node_type::less_than:
		result = inputs[0] < inputs[1] ? 1.0 : 0.0;
		break;
	case node_type::less_eq:
		result = inputs[0] <= inputs[1] ? 1.0 : 0.0;
		break;
	case node_type::larger_than:
		result = inputs[0] > inputs[1] ? 1.0 : 0.0;
		break;
	case node_type::larger_eq:
		result = inputs[0] >= inputs[1] ? 1.0 : 0.0;
		break;
	case node_type::equals:
		result = inputs[0] == inputs[1] ? 1.0 : 0.0;
		break;
	case node_type::not_equal:
		result = inputs[0] == inputs[1] ? 0.0 : 1.0;
		break;
	case node_type::logic_not:
		result = inputs[0] > 0.5 ? 0.0 : 1.0;
		break;
	case node_type::logic_and:
		result = inputs[0] * inputs[1] > 0.5 ? 1.0 : 0.0;
		break;
	case node_type::logic_or:
		result = inputs[0] + inputs[1] > 0.5 ? 1.0 : 0.0;
		break;
	case node_type::pow:
		result = std::pow(inputs[0], inputs[1]);
		break;
	case node_type::max:
		result = inputs[0];
		for (auto one_val : inputs)
		{
			if (one_val > result)
			{
				result = one_val;
			}
		}
		break;
	case node_type::min:
		result = inputs[0];
		for (auto one_val : inputs)
		{
			if (one_val < result)
			{
				result = one_val;
			}
		}
		break;
	case node_type::average:
		result = 0.0;
		for (auto one_val : inputs)
		{
			result += one_val;
		}
		result /= inputs.size();
	case node_type::percent_add:
		result = (1 + inputs[0] / 100.0) *inputs[1];
		break;
	case node_type::clamp:
		double a = inputs[0];
		double b = inputs[1];
		double c = inputs[2];
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
	default:
		result = value;
		break;
	}
	if (result == value)
	{
		return false;
	}
	else
	{
		value = result;
		return true;
	}
}

std::string cacl_node_base::print_formula(const std::string& my_name, const std::vector<std::string>& arg_names) const
{
	switch (cacl_type)
	{
	case node_type::root:
		return my_name + " = " + arg_names[0];
		break;
	case node_type::literal:
		return my_name + " = " + std::to_string(value);
	case node_type::import:
		return my_name + " = " + std::to_string(value);
	case node_type::input:
		return my_name + " = " + std::to_string(value);
	case node_type::neg:
		return my_name + " = -" + arg_names[0];
	case node_type::add:
		return my_name + " = " arg_names[0] + "+" + arg_names[1];
	case node_type::dec:
		return my_name + " = " arg_names[0] + "+" + arg_names[1];
	case node_type::mul:
		return my_name + " = " arg_names[0] + "*" + arg_names[1];
	case node_type::div:
		return my_name + " = " arg_names[0] + "/" + arg_names[1];
	case node_type::random:
		return my_name + " = random(" arg_names[0] + "," + arg_names[1] + ")";
	case node_type::condition:
		return my_name + " = " arg_names[0] + ">0.5?" + arg_names[1] +":" arg_names[2];
	case node_type::less_than:
		return my_name + " = " + arg_names[0] + "<" +arg_names[1] +"?1:0";
	case node_type::less_eq:
		return my_name + " = " + arg_names[0] + "<=" +arg_names[1] +"?1:0";
	case node_type::larger_than:
		return my_name + " = " + arg_names[0] + ">" +arg_names[1] +"?1:0";
	case node_type::larger_eq:
		return my_name + " = " + arg_names[0] + ">=" +arg_names[1] +"?1:0";
	case node_type::equals:
		return my_name + " = " + arg_names[0] + "==" +arg_names[1] +"?1:0";
	case node_type::not_equal:
		return my_name + " = " + arg_names[0] + "!=" +arg_names[1] +"?1:0";
	case node_type::logic_not:
		return my_name + " = " + arg_names[0] + "<0.5?1:0";
	case node_type::logic_and:
		return my_name + " = " + arg_names[0] + ">0.5&&" + arg_names[1] + ">0.5?1:0";
	case node_type::logic_or:
		return my_name + " = " + arg_names[0] + ">0.5||" + arg_names[1] + ">0.5?1:0";
	case node_type::pow:
		return my_name + " = " + "pow(" arg_names[0] + "," + arg_names[1] + ")";
	case node_type::max:
	{
		std::string result = my_name + " = max(";
		for(auto one_arg: arg_names)
		{
			result+=one_arg + ",";
		}
		result += ")";
		return result;
	}
	case node_type::min:
	{
		std::string result = my_name + " = min(";
		for(auto one_arg: arg_names)
		{
			result+=one_arg + ",";
		}
		result += ")";
		return result;
	}
	case node_type::average:
	{
		std::string result = my_name + " = average(";
		for(auto one_arg: arg_names)
		{
			result+=one_arg + ",";
		}
		result += ")";
		return result;
	}
	case node_type::clamp:
		return my_name + " = " + "clang(" arg_names[0] + "," + arg_names[1] + ","+ arg_names[2] + ")";
	case node_type::percent_add:
		return my_name + " = " + "(1 +" + arg_names[0] + "/100)*" + arg_names[1];
	default:
		return my_name;
	}
}