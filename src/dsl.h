#ifndef DSL_H
#define DSL_H

#define op_ctor(node, OP)                                               \
   node_op_ctor(node, (node)->left, (node)->right, (node)->prev, OP);

#define num_ctor(node, dbl)                                             \
   node_num_ctor(node, (node)->prev, dbl);

#define var_ctor(node)                                                  \
   node_var_ctor(node, (node)->prev);

#define undef_ctor(node)                                                \
   node_undef_ctor(node, (node)->prev);

#define dL(node) diff_execute((node)->left )
#define dR(node) diff_execute((node)->right)
#define cL(node) Tree_copy   ((node)->left )
#define cR(node) Tree_copy   ((node)->right)

#define Add(left, right) new_node_op(left, right, nullptr, OP_ADD)
#define Sub(left, right) new_node_op(left, right, nullptr, OP_SUB)
#define Mul(left, right) new_node_op(left, right, nullptr, OP_MUL)
#define Div(left, right) new_node_op(left, right, nullptr, OP_DIV)

#endif //DSL_H