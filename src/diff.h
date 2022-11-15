#ifndef DIFF_H
#define DIFF_H

enum TYPE_NODE
{
    NODE_UNDEF  ,
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
    Tree_node * prev;

    union
    {
        double      dbl;
        TYPE_OP      op;
        const char *var;
    }
    value;
};

const Tree_node default_node = 
{
    NODE_UNDEF  , // TYPE_NODE

    nullptr     , // left
    nullptr     , // right
    nullptr     , // prev

        0.0       // dbl
};

/*______________________________________FUNCTIONS_______________________________________*/

void        node_op_ctor        (Tree_node *const node, Tree_node *const  left,
                                                        Tree_node *const right,
                                                        Tree_node *const  prev,
                                                        TYPE_OP          value);
void        node_num_ctor       (Tree_node *const node, Tree_node *const  prev,
                                                        const double     value);
void        node_var_ctor       (Tree_node *const node, Tree_node *const  prev);
void        node_undef_ctor     (Tree_node *const node, Tree_node *const  prev);

Tree_node  *new_node_op         (TYPE_OP value,      Tree_node *const prev);
Tree_node  *new_node_num        (const double value, Tree_node *const prev);
Tree_node  *new_node_var        (                    Tree_node *const prev);
Tree_node  *new_node_undef      (                    Tree_node *const prev);

void        node_dtor           (Tree_node *const node);
void        Tree_dtor           (Tree_node *const root);

bool        Tree_parsing_main   (Tree_node *const root, const char *file);

/*______________________________________________________________________________________*/

#endif //DIFF_H