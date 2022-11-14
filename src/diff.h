#ifndef DIFF_H
#define DIFF_H

//>>>>>>>>>>>>>>>>>>>>>>>
#define NODE(name, ...) \
        NODE_##name ,
//<<<<<<<<<<<<<<<<<<<<<<<

enum TYPE_NODE
{
    #include "type_node.h"
};

//<<<<<<<<<<<<<<<<<<<<<<<
#undef NODE
//<<<<<<<<<<<<<<<<<<<<<<<

//>>>>>>>>>>>>>>>>>>>>>>>
#define OP(name, ...)   \
        OP_##name   ,
//<<<<<<<<<<<<<<<<<<<<<<<

enum TYPE_OP
{
    #include "type_op.h"
};

//>>>>>>>>>>>>>>>>>>>>>>>
#undef OP
//<<<<<<<<<<<<<<<<<<<<<<<

struct Tree_node
{
    TYPE_NODE type;

    Tree_node *left;
    Tree_node *right;

    union
    {
        double  num_dbl;
        TYPE_OP      op;
        const char *var;
    }
    value;
};

#endif //DIFF_H