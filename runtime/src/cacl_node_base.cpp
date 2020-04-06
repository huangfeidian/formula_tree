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