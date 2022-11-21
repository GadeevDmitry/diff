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

#define is_left_subtree(node)  node->prev->left  == node
#define is_right_subtree(node) node->prev->right == node

#define dL(node) diff_execute((node)->left )
#define dR(node) diff_execute((node)->right)
#define cL(node) Tree_copy   ((node)->left )
#define cR(node) Tree_copy   ((node)->right)

#define  Add(left, right) new_node_op (left, right, nullptr, OP_ADD )
#define  Sub(left, right) new_node_op (left, right, nullptr, OP_SUB )
#define  Mul(left, right) new_node_op (left, right, nullptr, OP_MUL )
#define  Div(left, right) new_node_op (left, right, nullptr, OP_DIV )
#define  Pow(left, right) new_node_op (left, right, nullptr, OP_POW )
#define  Log(left, right) new_node_op (left, right, nullptr, OP_LOG )
#define  Sin(left, right) new_node_op (left, right, nullptr, OP_SIN )
#define  Cos(left, right) new_node_op (left, right, nullptr, OP_COS )
#define Sqrt(left, right) new_node_op (left, right, nullptr, OP_SQRT)
#define   Sh(left, right) new_node_op (left, right, nullptr, OP_SH  )
#define   Ch(left, right) new_node_op (left, right, nullptr, OP_CH  )

#define Nul               new_node_num(0  ,         nullptr         )
#define Num(num)          new_node_num(num,         nullptr         )

#define  op(node)      (node)->value.op
#define dbl(node)      (node)->value.dbl
#define var(node)      (node)->value.var

#define   L(node)      (node)->left
#define   R(node)      (node)->right
#define   P(node)      (node)->prev

#endif //DSL_H