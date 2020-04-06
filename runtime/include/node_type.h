#pragma once
namespace spiritsaway::formula_tree::runtime
{
    enum class node_type
    {
		root,
        literal,
        import,
        input,
        neg,
        add,
        dec,
        mul,
        div,
        random, 
        condition,
        less_than,
        less_eq,
        larger_than,
        larger_eq,
        equals,
        not_equal,
        logic_not,
        logic_or,
        logic_and,
        pow,
        max,
        min,
        average,
    };
}
