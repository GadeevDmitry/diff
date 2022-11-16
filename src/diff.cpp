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

/*___________________________STATIC_FUNCTIONS__________________________*/

static void dfs_dtor(Tree_node *const node);

static bool Tree_parsing_execute(Tree_node *const root, const char *data     ,
                                                        const int   data_size,
                                                        int *const  data_pos );

static bool push_next_node      (Tree_node **node, Tree_node *const root);
static bool push_prev_node      (Tree_node **node, Tree_node *const root);

static bool put_dbl             (Tree_node *const node, const char *data     ,
                                                        int *const  data_pos );
static bool get_dbl             (double *const dbl,     const char *data     ,
                                                        int *const  data_pos );

static bool put_var             (Tree_node *const node);
static bool put_op              (Tree_node *const node, const char possible_op);

static void Tree_dump_dfs       (Tree_node *node, int *const node_number, FILE *const stream);
static void Tree_node_describe  (Tree_node *node, int *const node_number, FILE *const stream);
static void print_Tree_node     (Tree_node *node, int *const node_number, FILE *const stream,   GRAPHVIZ_COLOR fillcolor,
                                                                                                GRAPHVIZ_COLOR     color, 
                                                                                                const char    *node_type,
                                                                                                const char        *value);
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
    Tree_node *new_left  = (Tree_node *) log_calloc(1, sizeof(Tree_node));
    Tree_node *new_right = (Tree_node *) log_calloc(1, sizeof(Tree_node));

    if (new_node  == nullptr ||
        new_left  == nullptr ||
        new_right == nullptr)
    {
        log_free(new_node );
        log_free(new_left );
        log_free(new_right);

        return nullptr;
    }

    node_op_ctor(new_node, new_left ,
                           new_right,
                                prev,
                               value);
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

/*_____________________________________________________________________*/

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
    assert(root != nullptr);
    assert(file != nullptr);

    int         data_size = 0;
    int         data_pos  = 0;
    const char *data      = (char *) read_file(file, &data_size);
    
    if (data == nullptr)
    {
        log_error("Can't open the file\n");
        return false;
    }

    if (!Tree_parsing_execute(root, data     ,
                                    data_size,
                                   &data_pos  ))
    {
        log_error("Syntax_error in log_file.\n");
        
        log_free((char *) data);
        return false;
    }
    log_free((char *) data);
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
            log_error("Redefinition of node.\n");
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

    double dbl =             0;
   *data_pos   = *data_pos - 1;
    
    if (get_dbl(&dbl,   data,
                        data_pos ))
    {
        node_num_ctor(node, node->prev, dbl);
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

    node_var_ctor(node, node->prev);
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
    switch(possible_op)
    {
        case '+': node_op_ctor(node, node->left, node->right, node->prev, OP_ADD);
                  break;

        case '-': node_op_ctor(node, node->left, node->right, node->prev, OP_SUB);
                  break;

        case '*': node_op_ctor(node, node->left, node->right, node->prev, OP_MUL);
                  break;
        
        case '/': node_op_ctor(node, node->left, node->right, node->prev, OP_DIV);
                  break;

        default: log_error("Undefined operation.\n");
                 return false;
    }

    return true;
}

/*_____________________________________________________________________*/

void Tree_dump(Tree_node *root)
{
    assert(root);
    
    static int cur = 0;

    char    dump_txt[graph_size_file] = "";
    char    dump_png[graph_size_file] = "";

    sprintf(dump_txt, "dump_txt/Tree%d.txt", cur);
    sprintf(dump_png, "dump_png/Tree%d.png", cur);

    FILE *stream_txt =  fopen(dump_txt, "w");
    if   (stream_txt == nullptr)
    {
        log_error("Can't open dump file.\n");
        return;
    }

    setvbuf(stream_txt, nullptr, _IONBF, 0);
    fprintf(stream_txt, "digraph {\n"
                        "splines=ortho\n"
                        "node[shape=record, style=\"rounded, filled\", fontsize=8]\n");
    
    int node_number = 0;
    Tree_dump_dfs(root, &node_number, stream_txt);

    fprintf(stream_txt, "}\n");

    char cmd[graph_size_cmd] = "";
    sprintf    (cmd, "dot %s -T png -o %s", dump_txt, dump_png);
    system     (cmd);
    log_message("<img src=%s>\n", dump_png);

    fclose(stream_txt);
}

static void Tree_dump_dfs(Tree_node *node, int *const node_number, FILE *const stream)
{
    assert(node);
    assert(stream);

    int number_cur =  *node_number;
    Tree_node_describe(node, node_number, stream);

    int number_left  = *node_number;
    if (node->left)  Tree_dump_dfs(node->left, node_number, stream);

    int number_right = *node_number;
    if (node->right) Tree_dump_dfs(node->right, node_number, stream);

    if (node->left ) fprintf(stream, "node%d->node%d[xlabel=\"left \", color=\"black\"]\n", number_cur, number_left );
    if (node->right) fprintf(stream, "node%d->node%d[xlabel=\"right\", color=\"black\"]\n", number_cur, number_right);
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
                        value[0]  =        'x';
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