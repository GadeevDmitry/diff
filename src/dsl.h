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

#define L(node) (node)->left
#define R(node) (node)->right


#endif //DSL_H