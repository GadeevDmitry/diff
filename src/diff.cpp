#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>

#include "diff.h"
#include "dsl.h"

/*_____________________________________________________________________*/

void node_op_ctor(Tree_node *const node,    TYPE_OP                value,
                                            const Tree_node *const  left,
                                            const Tree_node *const right)
{
    assert(node  != nullptr);
    assert(left  != nullptr);
    assert(right != nullptr);

    node->type     = NODE_OP;
    node->left     =    left;
    node->right    =   right;
    node->value.op =   value;
}

void node_num_ctor(Tree_node *const node,   const double value)
{
    assert(node != nullptr);
    
    node->type      = NODE_NUM;
    node->left      =  nullptr;
    node->right     =  nullptr;
    node->value.dbl =    value;
}

void node_var_ctor(Tree_node *const node)
{
    assert(node != nullptr);

    node->type  = NODE_VAR;
    node->left  =  nullptr;
    node->right =  nullptr;
}

/*_____________________________________________________________________*/

Tree_node *new_node_op(TYPE_OP value)
{
    Tree_node *new_node  = (Tree_node *) calloc(1, sizeof(Tree_node));
    Tree_node *new_left  = (Tree_node *) calloc(1, sizeof(Tree_node));
    Tree_node *new_right = (Tree_node *) calloc(1, sizeof(Tree_node));

    if (new_node  == nullptr ||
        new_left  == nullptr ||
        new_right == nullptr)
    {
        free(new_node );
        free(new_left );
        free(new_right);

        return nullptr;
    }

    node_op_ctor(new_node, value, left, right);
    return       new_node;
}

Tree_node *new_node_num(const double value)
{
    Tree_node *new_node = (Tree_node *) calloc(1, sizeof(Tree_node));
    if        (new_node == nullptr) return nullptr;

    node_num_ctor(new_node);
    return        new_node ;
}

Tree_node *new_node_var()
{
    Tree_node *new_node = (Tree_node *) calloc(1, sizeof(Tree_node));
    if        (new_node == nullptr) return nullptr;

    node_var_ctor(new_node);
    return        new_node ;
}

/*_____________________________________________________________________*/
