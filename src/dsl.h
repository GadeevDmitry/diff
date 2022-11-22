#ifndef DSL_H
#define DSL_H

#define op_ctor(node, OP)                                               \
   node_op_ctor(node, OP, (node)->left, (node)->right, (node)->prev);

#define num_ctor(node, dbl)                                             \
   node_num_ctor(node, dbl, (node)->prev);

#define var_ctor(node, var)                                             \
   node_var_ctor(node, var, (node)->prev);

#define undef_ctor(node)                                                \
   node_undef_ctor(node, (node)->prev);

#define is_left_subtree(node)  node->prev->left  == node
#define is_right_subtree(node) node->prev->right == node

#define dL(node, var, d_mode) diff_execute((node)->left , var, d_mode)
#define dR(node, var, d_mode) diff_execute((node)->right, var, d_mode)
#define cL(node)              Tree_copy   ((node)->left )
#define cR(node)              Tree_copy   ((node)->right)

#define  Add(left, right) new_node_op (OP_ADD , left, right)
#define  Sub(left, right) new_node_op (OP_SUB , left, right)
#define  Mul(left, right) new_node_op (OP_MUL , left, right)
#define  Div(left, right) new_node_op (OP_DIV , left, right)
#define  Pow(left, right) new_node_op (OP_POW , left, right)
#define  Log(left, right) new_node_op (OP_LOG , left, right)
#define  Sin(left, right) new_node_op (OP_SIN , left, right)
#define  Cos(left, right) new_node_op (OP_COS , left, right)
#define Sqrt(left, right) new_node_op (OP_SQRT, left, right)
#define   Sh(left, right) new_node_op (OP_SH  , left, right)
#define   Ch(left, right) new_node_op (OP_CH  , left, right)

#define Nul               new_node_num(0)
#define Num(num)          new_node_num(num)

#define        op(node) (node)->value.op
#define       dbl(node) (node)->value.dbl
#define       var(node) (node)->value.var
#define tree_vars(node) (node)->tree_vars

#define   l(node) (node)->left
#define   r(node) (node)->right
#define   p(node) (node)->prev

#endif //DSL_H