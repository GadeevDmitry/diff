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

    Tree_node * left;
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

void        node_op_ctor    (Tree_node *const node, TYPE_OP          value,
                                                    Tree_node *const  left,
                                                    Tree_node *const right);
void        node_num_ctor   (Tree_node *const node,     const double value);
void        node_var_ctor   (Tree_node *const node);

Tree_node  *new_node_op     (TYPE_OP      value);
Tree_node  *new_node_num    (const double value);
Tree_node  *new_node_var    ();

/*______________________________________________________________________________________*/

#endif //DIFF_H