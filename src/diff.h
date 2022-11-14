#ifndef DIFF_H
#define DIFF_H

enum TYPE_NODE
{
    NODE_OP     ,
    NODE_NUM    ,
    NODE_VAR    ,
};

//>>>>>>>>>>>>>>>>>>>>>>>
#define OP(name, ...)   \
        OP_##name ,
//<<<<<<<<<<<<<<<<<<<<<<<

enum TYPE_OP
{
    #include "dsl.h"
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