#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>

#include "diff.h"
#include "dsl.h"

/*_____________________________________________________________________*/

void Tree_node_ctor(Tree_node *const node,  TYPE_NODE type  ,
                                            
                                            Tree_node *left ,
                                            Tree_node *right,
                                            
                                            void      *value)
{
    assert(node != nullptr);

    node->type  = type;
    node->left  = left;
    node->right = right;

    switch(type)
    {
        case NODE_OP:   node->value.op  = (TYPE_OP)      value;
                        break;

        case NODE_NUM:  node->value.dbl = (double)       value;
                        break;

        case NODE_VAR:  node->value.var = (const char *) value;
                        break;
    }
}

Tree_node *new_Tree_node(TYPE_NODE type,
                    
                         Tree_node *left,
                         Tree_node *right,
                    
                         void      *value)
{
    Tree_node *new_node = (Tree_node *) calloc(1, sizeoff(Tree_node));
    if        (new_node == nullptr) return new_node;

    Tree_node_ctor(new_node, type, left, right, value);
    return         new_node;
}

/*_____________________________________________________________________*/

