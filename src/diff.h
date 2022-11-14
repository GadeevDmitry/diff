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

void        Tree_node_ctor          (Tree_node *const node, TYPE_NODE type  ,
                                            
                                                            Tree_node *left ,
                                                            Tree_node *right,
                                            
                                                            void      *value);

Tree_node  *new_Tree_node           (                       TYPE_NODE type,
                    
                                                            Tree_node *left,
                                                            Tree_node *right,
                    
                                                            void      *value);
/*______________________________________________________________________________________*/

#endif //DIFF_H