#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>

#include "diff.h"
#include "dsl.h"

#include "../lib/logs/log.h"
#include "../lib/read_write/read_write.h"
#include "../lib/algorithm/algorithm.h"
#include "../lib/graph_dump/graph_dump.h"

/*___________________________STATIC_FUNCTION___________________________*/

static bool Tree_verify             (                           Tree_node *const root);
static void Tree_verify_dfs         (unsigned int *const err,   Tree_node *const root,
                                                                Tree_node *const node);
static void print_error_messages    (unsigned int        err);

static void dfs_dtor                (Tree_node *const node);

static bool Tree_parsing_execute    (Tree_node *const root, const char *data     ,
                                                            const int   data_size,
                                                            int *const  data_pos );

static bool push_next_node          (Tree_node **node, Tree_node *const root);
static bool push_prev_node          (Tree_node **node, Tree_node *const root);

static bool put_dbl                 (Tree_node *const node, const char *data     ,
                                                            int *const  data_pos );
static bool get_dbl                 (double *const dbl,     const char *data     ,
                                                            int *const  data_pos );

static bool put_var                 (Tree_node *const node);
static bool put_op                  (Tree_node *const node, const char possible_op);

static void diff_execute            (Tree_node *node   , Tree_node *diff_node);
static void Tree_copy               (Tree_node *cp_from, Tree_node *cp_to    );

static void Tree_dump_graphviz_dfs  (Tree_node *node, int *const node_number, FILE *const stream);
static void Tree_node_describe      (Tree_node *node, int *const node_number, FILE *const stream);
static void print_Tree_node         (Tree_node *node, int *const node_number, FILE *const stream,   GRAPHVIZ_COLOR fillcolor,
                                                                                                    GRAPHVIZ_COLOR     color, 
                                                                                                    const char    *node_type,
                                                                                                    const char        *value);
static void Tree_dump_txt_dfs       (Tree_node *node, bool bracket);

/*___________________________STATIC_CONST______________________________*/

static const Tree_node default_node = 
{
    NODE_UNDEF  , // TYPE_NODE

    nullptr     , // left
    nullptr     , // right
    nullptr     , // prev

        0.0       // dbl
};

static const int op_priority[] =
{
    1   , // OP_ADD
    1   , // OP_SUB
    2   , // OP_MUL
    2   , // OP_DIV
};

static const char *op_names[] =
{
    "+" , // OP_ADD
    "-" , // OP_SUB
    "*" , // OP_MUL
    "/" , // OP_DIV
};

static const int VALUE_SIZE = 100;

enum VERIFY_ERROR
{
    NULLPTR_ROOT        ,
    NULLPTR_PREV        ,
    NULLPTR_LEFT        ,
    NULLPTR_RIGHT       ,

    UNDEF_TYPE_NODE     ,
    
    TERMINAL_OP         ,
    NON_TERMINAL_VAR    ,
    NON_TERMINAL_NUM    ,
};

static const char *verify_error_messages[] =
{
    "Root  is nullptr.\n"                   ,
    "Prev  is nullptr.\n"                   ,
    "Left  is nullptr.\n"                   ,
    "Right is nullptr.\n"                   ,

    "Type of node is undef.\n"              ,

    "Operation-type node is     terminal.\n",
    " Variable-type node is not terminal.\n",
    "   Number-type node is not terminal.\n",
};

/*_____________________________________________________________________*/

static bool Tree_verify(Tree_node *const root)
{
    unsigned int err = 0;

    if (root == nullptr)  err = 1 << NULLPTR_ROOT;
    else Tree_verify_dfs(&err, root, root);
    
    print_error_messages(err);

    return err == 0;
}

static void Tree_verify_dfs(unsigned int *const err,    Tree_node *const root,
                                                        Tree_node *const node)
{
    assert(node != nullptr);
    assert(root != nullptr);

    if (node->left ) Tree_verify_dfs(err, root, node->left );
    if (node->right) Tree_verify_dfs(err, root, node->right);

    bool is_terminal_node = false;

    if (node->right == nullptr &&
        node->left  == nullptr   ) is_terminal_node = true;

    if (node->prev  == nullptr && node != root) (*err) = (*err) | (1 << NULLPTR_PREV   );
    if (node->left  == nullptr && node->right ) (*err) = (*err) | (1 << NULLPTR_LEFT   );
    if (node->right == nullptr && node->left  ) (*err) = (*err) | (1 << NULLPTR_RIGHT  );
    
    if (node->type  == NODE_UNDEF)
    {
        (*err) = (*err) | (1 << UNDEF_TYPE_NODE);
        return;
    }
    
    if      (node->type == NODE_OP  &&  is_terminal_node) (*err) = (*err) | (1 << TERMINAL_OP     );
    else if (node->type == NODE_NUM && !is_terminal_node) (*err) = (*err) | (1 << NON_TERMINAL_NUM);
    else if (node->type == NODE_VAR && !is_terminal_node) (*err) = (*err) | (1 << NON_TERMINAL_VAR);
}

static void print_error_messages(unsigned int err)
{
    if (err == 0)
    {
        log_message(GREEN "Tree is OK.\n" CANCEL);
        return;
    }

    log_error("Tree_verify failed.\n");

    for (long unsigned shift = 0; sizeof(char *) * shift < sizeof(verify_error_messages); ++shift)
    {
        if (err & (1 << shift)) log_error(verify_error_messages[shift]);
    }
}

/*_____________________________________________________________________*/

void node_op_ctor(Tree_node *const node,    Tree_node *const  left,
                                            Tree_node *const right,
                                            Tree_node *const  prev,
                                            TYPE_OP          value)
{
    assert(node  != nullptr);
    assert(left  != nullptr);
    assert(right != nullptr);

    node->type     = NODE_OP;

    node->left     =    left;
    node->right    =   right;
    node->prev     =    prev;

    node->value.op =   value;
}

void node_num_ctor(Tree_node *const node,   Tree_node *const prev,
                                            const double    value)
{
    assert(node != nullptr);

    node->type      = NODE_NUM;
    
    node->left      =  nullptr;
    node->right     =  nullptr;
    node->prev      =     prev;

    node->value.dbl =    value;
}

void node_var_ctor(Tree_node *const node, Tree_node *const prev)
{
    assert(node != nullptr);

    node->type      = NODE_VAR;
    
    node->left      =  nullptr;
    node->right     =  nullptr;
    node->prev      =     prev;

    node->value.var =      "x";
}

void node_undef_ctor(Tree_node *const node, Tree_node *const prev)
{
    assert (node != nullptr);

   *node       = default_node;
    node->prev =         prev;
}

/*_____________________________________________________________________*/

Tree_node *new_node_op(TYPE_OP value, Tree_node *const prev)
{
    Tree_node *new_node  = (Tree_node *) log_calloc(1, sizeof(Tree_node));
    Tree_node *new_left  = new_node_undef(new_node);
    Tree_node *new_right = new_node_undef(new_node);

    if (new_node  == nullptr ||
        new_left  == nullptr ||
        new_right == nullptr)
    {
        log_message("log_calloc returns nullptr in %s.\n", __PRETTY_FUNCTION__);

        log_free(new_node );
        log_free(new_left );
        log_free(new_right);

        return nullptr;
    }

    node_op_ctor(new_node, new_left ,
                           new_right,
                           prev     ,
                           value    );
    return       new_node;
}

Tree_node *new_node_num(const double value, Tree_node *const prev)
{
    Tree_node *new_node = (Tree_node *) log_calloc(1, sizeof(Tree_node));
    if        (new_node == nullptr) return nullptr;

    node_num_ctor(new_node, prev,
                           value);
    return        new_node;
}

Tree_node *new_node_var(Tree_node *const prev)
{
    Tree_node *new_node = (Tree_node *) log_calloc(1, sizeof(Tree_node));
    if        (new_node == nullptr) return nullptr;

    node_var_ctor(new_node, prev);
    return        new_node;
}

Tree_node *new_node_undef(Tree_node *const prev)
{
    Tree_node *new_node = (Tree_node *) log_calloc(1, sizeof(Tree_node));
    if        (new_node == nullptr) return nullptr;

    node_undef_ctor(new_node, prev);
    return          new_node;
}

/*_____________________________________________________________________*/

void node_dtor(Tree_node *const node)
{
    log_free(node);
}

void Tree_dtor(Tree_node *const root)
{
    if (root == nullptr) return;

    dfs_dtor(root);
}

static void dfs_dtor(Tree_node *const node)
{
    assert(node != nullptr);

    if (node->left  != nullptr) dfs_dtor(node->left );
    if (node->right != nullptr) dfs_dtor(node->right);

    node_dtor(node);
}

/*_____________________________________________________________________*/

bool Tree_parsing_main(Tree_node *const root, const char *file)
{
    log_header(__PRETTY_FUNCTION__);

    assert(file != nullptr);

    int         data_size = 0;
    int         data_pos  = 0;
    const char *data      = (char *) read_file(file, &data_size);
    
    if (data == nullptr)
    {
        log_error     ("Can't open the file\n");
        log_end_header();
        return false;
    }

    if (!Tree_parsing_execute(root, data     ,
                                    data_size,
                                   &data_pos  ))
    {
        log_error("Syntax_error in download file.\n");
        
        log_free      ((char *) data);
        log_end_header();
        return false;
    }
    log_free      ((char *) data);
    log_message   (GREEN "Parsing successful.\n" CANCEL);
    log_end_header();
    return true;
}

static bool Tree_parsing_execute(Tree_node *const root, const char *data     ,
                                                        const int   data_size,
                                                        int *const  data_pos )
{
    assert(root);
    assert(data);
    assert(data_pos);

    Tree_node *cur_node = nullptr;
    skip_spaces(data, data_size, data_pos);

    while (*data_pos < data_size && data[*data_pos] != '\0')
    {
        char cur_char  = data[*data_pos];
        *data_pos      =  1 + *data_pos ;

        if      (cur_char == '('   ) { if (!push_next_node(&cur_node, root     )) return false; }
        else if (cur_char == ')'   ) { if (!push_prev_node(&cur_node, root     )) return false; }
        else if (isdigit(cur_char) ) { if (!put_dbl       ( cur_node, data     ,
                                                                      data_pos )) return false; }
        else if (cur_char == 'x'   ) { if (!put_var       ( cur_node           )) return false; }
        else                         { if (!put_op        ( cur_node, cur_char )) return false; }

        skip_spaces(data, data_size, data_pos);
    }

    if (cur_node != nullptr)
    {
        log_error("Finishes not in nullptr-value.\n");
        return false;
    }
    return true;
}

static bool push_next_node(Tree_node **node, Tree_node *const root)
{
    assert(root != nullptr);
    assert(node != nullptr);

    if (*node == nullptr)
    {
       *node = root;
        return true;
    }

    if ((*node)->left != nullptr)
    {
        if ((*node)->right->type == NODE_UNDEF)
        {
            *node = (*node)->right;
            return true;
        }
        else
        {
            log_error("Pushing in the third time.\n");
            return false;
        }
    }

    (*node)->left  = new_node_undef(*node);
    (*node)->right = new_node_undef(*node);
    
    if ((*node)->left  == nullptr ||
        (*node)->right == nullptr)
    {
        log_free((*node)->left );
        log_free((*node)->right);

        log_error("Can't create new node.\n");
        return false;
    }

    *node = (*node)->left;
    return true;
}

static bool push_prev_node(Tree_node **node, Tree_node *const root)
{
    assert(root != nullptr);
    assert(node != nullptr);

    if (*node == nullptr)
    {
        log_error("Can't get previos node.\n");
        return false;
    }

   *node = (*node)->prev;
   return true;
}

static bool put_dbl(Tree_node *const node,  const char *data     ,
                                            int *const  data_pos )
{
    assert(data     != nullptr);
    assert(data_pos != nullptr);
    if    (node     == nullptr)
    {
        log_error("Can't put double-value in nullptr-node.\n");
        return false;
    }
    if (node->type != NODE_UNDEF)
    {
        log_error("Redefinition of dbl-node.\n");
        return false;
    }

    double dbl =             0;
   *data_pos   = *data_pos - 1;
    
    if (get_dbl(&dbl,   data,
                        data_pos ))
    {
        num_ctor(node, dbl);
        return true;
    }

    log_error("Can't put invalid-double in the node.\n");
    return false;
}

static bool get_dbl(double *const dbl,  const char *data    ,
                                        int *const  data_pos)
{
    assert(dbl      != nullptr);
    assert(data     != nullptr);
    assert(data_pos != nullptr);

    char *dbl_end = nullptr;

    *dbl          = strtod(data + *data_pos, &dbl_end);
    *data_pos     = (int)             (dbl_end - data);

    if (*dbl == HUGE_VAL)
    {
        log_error("Double-value is HUGE_VAL.\n");
        return false;
    }
    return true;
}

static bool put_var(Tree_node *const node)
{
    if (node == nullptr)
    {
        log_error("Can't put variable in nullptr-node.\n");
        return false;
    }
    if (node->type != NODE_UNDEF)
    {
        log_error("Redefinition of var-node.\n");
        return false;
    }

    var_ctor(node);
    return true; 
}

static bool put_op(Tree_node *const node, const char possible_op)
{
    if (node == nullptr)
    {
        log_error("Can't put possible operation in nullptr-node.\n");
        return false;
    }
    if (node->left  == nullptr ||
        node->right == nullptr)
    {
        log_error("Can't put possible operation in terminal node.\n");
        return false;
    }
    if (node->type != NODE_UNDEF)
    {
        log_error("Redefinition of op-node.\n");
        return false;
    }

    switch(possible_op)
    {
        case '+': op_ctor(node, OP_ADD);
                  break;

        case '-': op_ctor(node, OP_SUB);
                  break;

        case '*': op_ctor(node, OP_MUL);
                  break;
        
        case '/': op_ctor(node, OP_DIV);
                  break;

        default: log_error("Undefined operation.\n");
                 return false;
    }

    return true;
}

/*_____________________________________________________________________*/

Tree_node *diff_main(Tree_node *root)
{
    log_header(__PRETTY_FUNCTION__);

    if (Tree_verify(root) == false)
    {
        log_error     ("Can't differentiate the function, because tree is invalid.\n");
        log_end_header();
        return nullptr;
    }

    Tree_node *diff_root = new_node_undef(nullptr);
    if        (diff_root == nullptr)
    {
        log_error     ("log_calloc returns nullptr.\n");
        log_end_header();
        return nullptr;
    }

    diff_execute  (root, diff_root);
    log_end_header();
    return diff_root;
}

static void diff_execute(Tree_node *node, Tree_node *diff_node)
{
    assert(node      != nullptr);
    assert(diff_node != nullptr);

    switch(node->type)
    {
        case NODE_NUM:  num_ctor(diff_node, 0);
                        break;
        
        case NODE_VAR:  num_ctor(diff_node, 1);
                        break;
        
        case NODE_OP:   switch (node->value.op)
                        {
                            case OP_ADD:    node_op_ctor(diff_node, new_node_undef(diff_node),
                                                                    new_node_undef(diff_node),
                                                                    diff_node->prev          ,
                                                                    OP_ADD                   );

                                            diff_execute(node->left , diff_node->left );
                                            diff_execute(node->right, diff_node->right);
                                            break;
                            
                            case OP_SUB:    node_op_ctor(diff_node, new_node_undef(diff_node),
                                                                    new_node_undef(diff_node),
                                                                    diff_node->prev          ,
                                                                    OP_SUB                   );

                                            diff_execute(node->left , diff_node->left );
                                            diff_execute(node->right, diff_node->right);
                                            break;
                            
                            case OP_MUL:    node_op_ctor(diff_node, new_node_op(OP_MUL, diff_node),
                                                                    new_node_op(OP_MUL, diff_node),
                                                                    diff_node->prev               ,
                                                                    OP_ADD                        );
                                            
                                            Tree_copy(node->left , diff_node->right->left );
                                            Tree_copy(node->right, diff_node->left ->right);

                                            diff_execute(node->left , diff_node->left ->left );
                                            diff_execute(node->right, diff_node->right->right);
                                            break;
                            
                            case OP_DIV:    node_op_ctor(diff_node, new_node_op(OP_SUB, diff_node),
                                                                    new_node_op(OP_MUL, diff_node),
                                                                    diff_node->prev               ,
                                                                    OP_DIV                        );

                                            node_op_ctor(diff_node->left->left , new_node_undef(diff_node->left->left) ,
                                                                                 new_node_undef(diff_node->left->left) ,
                                                                                 diff_node->left                       ,
                                                                                 OP_MUL                                );
                                            node_op_ctor(diff_node->left->right, new_node_undef(diff_node->left->right),
                                                                                 new_node_undef(diff_node->left->right),
                                                                                 diff_node->left                       ,
                                                                                 OP_MUL                                );

                                            Tree_copy(node->right, diff_node->right->left        );
                                            Tree_copy(node->right, diff_node->right->right       );
                                            Tree_copy(node->right, diff_node->left ->left ->right);
                                            Tree_copy(node->left , diff_node->left ->right->left );

                                            diff_execute(node->left , diff_node->left->left ->left );
                                            diff_execute(node->right, diff_node->left->right->right);
                                            break;

                            default     :   assert(false && "default case in TYPE_OP-switch");
                        }
                        break;
        
        case NODE_UNDEF:
        default        :   assert(false && "default case in TYPE_NODE-switch");
    }
}

static void Tree_copy(Tree_node *cp_from, Tree_node *cp_to)
{
    assert(cp_from != nullptr);
    assert(cp_to   != nullptr);

    switch (cp_from->type)
    {
        case NODE_NUM: node_num_ctor(cp_to, cp_to->prev, cp_from->value.dbl);
                       break;

        case NODE_VAR: node_var_ctor(cp_to, cp_to->prev);
                       break;

        case NODE_OP : node_op_ctor (cp_to, new_node_undef(cp_to),
                                            new_node_undef(cp_to),
                                            cp_to  ->prev        ,
                                            cp_from->value.op    );

                       Tree_copy(cp_from->left , cp_to->left );
                       Tree_copy(cp_from->right, cp_to->right);
                       break;

        case NODE_UNDEF:
        default        :Tree_dump_graphviz(cp_from);
                        assert(false && "Undefined node-type in cp_from.\n");
                        break;
    }
}

/*_____________________________________________________________________*/

void Tree_dump_graphviz(Tree_node *root)
{
    log_header  (__PRETTY_FUNCTION__);
    Tree_verify (root);

    if (root == nullptr)
    {
        log_warning   ("Nullptr-root passed in Tree_dump_graphviz.\n");
        log_end_header();
        return;
    }
    
    static int cur = 0;

    char    dump_txt[graph_size_file] = "";
    char    dump_png[graph_size_file] = "";

    sprintf(dump_txt, "dump_txt/Tree%d.txt", cur);
    sprintf(dump_png, "dump_png/Tree%d.png", cur);

    FILE *stream_txt =  fopen(dump_txt, "w");
    if   (stream_txt == nullptr)
    {
        log_error     ("Can't open dump file.\n");
        log_end_header();
        return;
    }
    ++cur;

    setvbuf(stream_txt, nullptr, _IONBF, 0);
    fprintf(stream_txt, "digraph {\n"
                        "splines=ortho\n"
                        "node[shape=record, style=\"rounded, filled\", fontsize=8]\n");
    
    int node_number = 0;
    Tree_dump_graphviz_dfs(root, &node_number, stream_txt);

    fprintf(stream_txt, "}\n");

    char cmd[graph_size_cmd] = "";
    sprintf       (cmd, "dot %s -T png -o %s", dump_txt, dump_png);
    system        (cmd);
    log_message   ("<img src=%s>\n", dump_png);
    log_end_header();

    fclose(stream_txt);

}

static void Tree_dump_graphviz_dfs(Tree_node *node, int *const node_number, FILE *const stream)
{
    assert(node);
    assert(stream);

    int number_cur =  *node_number;
    Tree_node_describe(node, node_number, stream);

    int number_left  = *node_number;
    if (node->left)  Tree_dump_graphviz_dfs(node->left, node_number, stream);

    int number_right = *node_number;
    if (node->right) Tree_dump_graphviz_dfs(node->right, node_number, stream);

    if (node->left ) fprintf(stream, "node%d->node%d[color=\"black\"]\n", number_cur, number_left );
    if (node->right) fprintf(stream, "node%d->node%d[color=\"black\"]\n", number_cur, number_right);
}

static void Tree_node_describe(Tree_node *node, int *const node_number, FILE *const stream)
{
    assert(node);
    assert(stream);

    GRAPHVIZ_COLOR fillcolor = WHITE;
    GRAPHVIZ_COLOR     color = WHITE;

    const char  *node_type = nullptr;
    char value[VALUE_SIZE] =      {};

    switch (node->type)
    {
        case NODE_VAR:      color =  DARK_BLUE;
                        fillcolor = LIGHT_BLUE;
                        node_type = "NODE_VAR";
                        sprintf(value, "var: x");
                        break;
        
        case NODE_NUM:      color =  DARK_GREEN;
                        fillcolor = LIGHT_GREEN;
                        node_type =  "NODE_NUM";
                        sprintf(value, "dbl: %lg", node->value.dbl);
                        break;

        case NODE_OP:       color =  DARK_ORANGE;
                        fillcolor = LIGHT_ORANGE;
                        node_type =    "NODE_OP";
                        sprintf(value, "op: %d", node->value.op);
                        break;

        case NODE_UNDEF:    color =    DARK_RED ;
                        fillcolor =   LIGHT_PINK;
                        node_type = "NODE_UNDEF";
                        value[0]  =          '?';
                        break;
        
        default:            color =             BLACK;
                        fillcolor =        LIGHT_GREY;
                        node_type = "UNDEF_NODE_TYPE";
                        value[0]  =               '?';
                        break;
    }
    print_Tree_node(node, node_number, stream,  fillcolor,
                                                    color,
                                                node_type,
                                                value    );

}

static void print_Tree_node(Tree_node *node, int *const node_number, FILE *const stream,    GRAPHVIZ_COLOR fillcolor,
                                                                                            GRAPHVIZ_COLOR     color, 
                                                                                            const char    *node_type,
                                                                                            const char        *value)
{
    assert(node      != nullptr);
    assert(node_type != nullptr);
    assert(value     != nullptr);

    fprintf(stream, "node%d[color=\"%s\", fillcolor=\"%s\", label=\"{cur = %p\\n | prev = %p\\n | type = %s\\n | %s | {left = %p | right = %p}}\"]\n",
                        *node_number,
                                    graphviz_color_names[color],
                                                      graphviz_color_names[fillcolor],
                                                                           node,
                                                                                          node->prev,
                                                                                                         node_type,
                                                                                                                 value,
                                                                                                                              node->left,
                                                                                                                                           node->right);
    *node_number = *node_number + 1;
}

/*_____________________________________________________________________*/

void Tree_dump_txt(Tree_node *root)
{
    log_header(__PRETTY_FUNCTION__);

    if (Tree_verify(root) == false)
    {
        log_message("Can't generate txt_dump, because tree is invalid.\n"
                    "Get graphviz_dump.\n");
        
        Tree_dump_graphviz(root);
        return;
    }

    Tree_dump_txt_dfs(root, false);
    log_end_header   ();
}

static void Tree_dump_txt_dfs(Tree_node *node, bool bracket)
{
    assert(node != nullptr);

    if (bracket) log_message("(");

    if      (node->type == NODE_NUM) log_message("%lg", node->value.dbl);
    else if (node->type == NODE_VAR) log_message("x");
    else
    {
        if (node->left->type != NODE_OP || op_priority[node->left->value.op] >= op_priority[node->value.op])
        {
            Tree_dump_txt_dfs(node->left, false);
        }
        else { Tree_dump_txt_dfs(node->left, true); }

        log_message(op_names[node->value.op]);

        if (node->right->type != NODE_OP || op_priority[node->right->value.op] > op_priority[node->value.op])
        {
            Tree_dump_txt_dfs(node->right, false);
        }
        else { Tree_dump_txt_dfs(node->right, true); }
    }

    if (bracket) log_message(")");
}

/*_____________________________________________________________________*/