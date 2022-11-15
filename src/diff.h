#ifndef DIFF_H
#define DIFF_H

enum TYPE_NODE
{
    NODE_OP     ,
    NODE_NUM    ,
    NODE_VAR    ,
};

enum TYPE_OP
{
    OP_ADD      ,
    OP_SUB      ,
    OP_MUL      ,
    OP_DIV      ,
};

struct Tree_node
{
    TYPE_NODE type;

    Tree_node *left;
    Tree_node *right;

    union
    {
        double      dbl;
        TYPE_OP      op;
        const char *var;
    }
    value;
};

/*_________________________________FUNCTION_DECLARATION_________________________________*/

/*______________________________________________________________________________________*/

#endif //DIFF_H