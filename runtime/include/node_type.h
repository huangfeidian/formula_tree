#pragma once
namespace spiritsaway::formula_tree::runtime
{
    enum class node_type
    {
		root,
        literal,
        import,
        input,
        add,
        dec,
        mul,
        div,
        pow,
        max,
        min,
        average,
		clamp,
		percent_add
    };
}
