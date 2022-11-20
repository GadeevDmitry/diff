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

static bool         Tree_verify             (                           Tree_node *const root);
static void         Tree_verify_dfs         (unsigned int *const err,   Tree_node *const root,
                                                                        Tree_node *const node);
static void         print_error_messages    (unsigned int        err);

static void         dfs_dtor                (Tree_node *const node);

static bool         Tree_parsing_execute    (Tree_node *const root, const char *data     ,
                                                                    const int   data_size,
                                                                    int *const  data_pos );

static bool         push_next_node          (Tree_node **node, Tree_node *const root);
static bool         push_prev_node          (Tree_node **node, Tree_node *const root);

static bool         put_dbl                 (Tree_node *const node, const char *data    ,
                                                                    int *const  data_pos);
static bool         get_dbl                 (double *const dbl,     const char *data    ,
                                                                    int *const  data_pos);

static bool         put_var                 (Tree_node *const node);
static bool         put_op                  (Tree_node *const node, const char *data     ,
                                                                    const int   data_size,
                                                                    int *const  data_pos );

static void         Tree_optimize_execute           (Tree_node **node);
static bool         Tree_optimize_numbers           (Tree_node *node);
static double       Tree_counter                    (const double left, const double right, TYPE_OP op);
static bool         Tree_optimize_add_main          (Tree_node **node);
static bool         Tree_optimize_sub_main          (Tree_node **node);
static bool         Tree_optimize_mul_main          (Tree_node **node);
static bool         Tree_optimize_div_main          (Tree_node **node);
static void         Tree_optimize_add_sub_mul_div   (Tree_node **node, Tree_node **null_son, Tree_node **good_son);

static Tree_node   *diff_execute            (Tree_node *const node);
static void         diff_prev_init          (Tree_node *diff_node, Tree_node *prev);
static Tree_node   *Tree_copy               (Tree_node *cp_from);

static void         Tree_dump_graphviz_dfs  (Tree_node *node, int *const node_number, FILE *const stream);
static void         Tree_node_describe      (Tree_node *node, int *const node_number, FILE *const stream);
static void         print_Tree_node         (Tree_node *node, int *const node_number, FILE *const stream,   GRAPHVIZ_COLOR fillcolor,
                                                                                                            GRAPHVIZ_COLOR     color, 
                                                                                                            const char    *node_type,
                                                                                                            const char        *value);
static void         Tree_dump_txt_dfs       (Tree_node *node, bool bracket);
static void         dump_txt_sin_cos        (Tree_node *node, bool bracket);
static void         Tree_dump_tex_system    (char *const dump_tex, char *const dump_pdf, const int cur);
static void         Tree_dump_tex_dfs       (Tree_node *node, bool bracket, FILE *const stream);
static void         Tree_dump_tex_op_sin_cos(Tree_node *node, FILE *const stream);
static void         Tree_dump_tex_op_div    (Tree_node *node, FILE *const stream);
static bool         Tree_dump_tex_op_sub    (Tree_node *node, FILE *const stream);

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
    4   , // OP_SIN
    4   , // OP_COS
    3   , // OP_POW
};

static const char *op_names[] =
{
    "+"     , // OP_ADD
    "-"     , // OP_SUB
    "*"     , // OP_MUL
    "/"     , // OP_DIV
    "sin"   , // OP_SIN
    "cos"   , // OP_COS
    "^"     , // OP_POW
};

static const int VALUE_SIZE = 100;
static const int  FILE_SIZE = 100;
static const int   CMD_SIZE = 300;
static const int PDF_WIDTH  = 500;
static const int PDF_HEIGHT = 500;

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

const char *tex_header =
"\\documentclass[12pt,a5paper,fleqn]{article}\n"
"\\usepackage[utf8]{inputenc}\n"
"\\usepackage{amssymb, amsmath, multicol}\n"
"\\usepackage[russian]{babel}\n"
"\\usepackage{graphicx}\n"
"\\usepackage[shortcuts,cyremdash]{extdash}\n"
"\\usepackage{wrapfig}\n"
"\\usepackage{floatflt}\n"
"\\usepackage{lipsum}\n"
"\\usepackage{concmath}\n"
"\\usepackage{euler}\n"
"\\usepackage{libertine}\n"
"\n"
"\\oddsidemargin=-15.4mm\n"
"\\textwidth=127mm\n"
"\\headheight=-32.4mm\n"
"\\textheight=277mm\n"
"\\tolerance=100\n"
"\\parindent=0pt\n"
"\\parskip=8pt\n"
"\\pagestyle{empty}\n"
"\n"
"\\usepackage[normalem]{ulem}\n"
"\\usepackage{mdframed}\n"
"\\usepackage{amsthm}\n"
"\n"
"\\flushbottom\n"
"\n"
"\\begin{document}\n"
;

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

    if (node->prev  == nullptr && node != root) (*err) = (*err) | (1 << NULLPTR_PREV );
    if (node->left  == nullptr && node->right ) (*err) = (*err) | (1 << NULLPTR_LEFT );
    if (node->right == nullptr && node->left  ) (*err) = (*err) | (1 << NULLPTR_RIGHT);

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

Tree_node *new_node_op(Tree_node *left ,
                       Tree_node *right,
                       Tree_node *prev ,
                       TYPE_OP    value)
{
    Tree_node *new_node = (Tree_node *) log_calloc(1, sizeof(Tree_node));
    if        (new_node == nullptr)
    {
        log_message("log_calloc returns nullptr in %s.\n", __PRETTY_FUNCTION__);
        return nullptr;
    }

    node_op_ctor(new_node, left ,
                           right,
                           prev ,
                           value);
    return new_node;
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

        if      (cur_char == '('   ) { if (!push_next_node(&cur_node, root      )) return false; }
        else if (cur_char == ')'   ) { if (!push_prev_node(&cur_node, root      )) return false; }
        else if (isdigit(cur_char) ) { if (!put_dbl       ( cur_node, data      ,
                                                                      data_pos  )) return false; }
        else if (cur_char == 'x'   ) { if (!put_var       ( cur_node            )) return false; }
        else                         { if (!put_op        ( cur_node, data,
                                                                      data_size,
                                                                      data_pos  )) return false; }

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

static bool put_dbl(Tree_node *const node,  const char *data    ,
                                            int *const  data_pos)
{
    assert(data     != nullptr);
    assert(data_pos != nullptr);
    if    (node     == nullptr)
    {
        log_error("Can't put double-value in nullptr-node.\n");
        return false;
    }
    if (node->left  != nullptr ||
        node->right != nullptr)
    {
        log_error("Can't put double-number in non-terminal node.\n");
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
    if (node->left  != nullptr ||
        node->right != nullptr)
    {
        log_error("Can't put variable in non-terminal mode");
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

static bool put_op(Tree_node *const node, const char *data     ,
                                          const int   data_size,
                                          int *const  data_pos )
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

    *data_pos = *data_pos - 1;

    char           possible_op [VALUE_SIZE] = {};
    get_word_split(possible_op, VALUE_SIZE, data, data_size, data_pos, "0123456789()x");

    for (size_t op_cnt = 0; sizeof(char *) * op_cnt != sizeof(op_names); ++op_cnt)
    {
        if (!strcasecmp(possible_op, op_names[op_cnt]))
        {
            op_ctor(node, (TYPE_OP)op_cnt);
            return true;
        }
    }

    log_error("Undefined operation.\n");
    return false;
}

/*_____________________________________________________________________*/

void Tree_optimize_main(Tree_node **root)
{
    log_header(__PRETTY_FUNCTION__);

    if (root == nullptr)
    {
        log_error     ("Nullptr-pointer to the tree to optimize.\n");
        log_end_header();
        return;
    }
    if (Tree_verify(*root) == false)
    {
        log_error     ("Can't optimize the tree, because it is invalid.\n");
        log_end_header();
        return;
    }

    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //log_message(__PRETTY_FUNCTION__);
    //;log_message("\nroot->type = %d\n\n", (*root)->type);
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

    Tree_optimize_execute(root);
    log_end_header       ();
}

static void Tree_optimize_execute(Tree_node **node)
{
    assert( node != nullptr);
    assert(*node != nullptr);

    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    /*
    log_message(__PRETTY_FUNCTION__);
    log_message("\nnode->type = %d\n", (*node)->type);
    if ((*node)->type == NODE_OP)
    {
        log_message("node->op_type = %d\n\n", (*node)->value.op);
    }
    else log_message("\n");
    */
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

    if ((*node)->type != NODE_OP) return;

    Tree_optimize_execute(&((*node)->left ));
    Tree_optimize_execute(&((*node)->right));

    if (Tree_optimize_numbers(*node)) return;
    if (Tree_optimize_add_main(node)) return;
    if (Tree_optimize_sub_main(node)) return;
    if (Tree_optimize_mul_main(node)) return;
    if (Tree_optimize_div_main(node)) return;
}

/*_____________________________________________________________________*/

static bool Tree_optimize_numbers(Tree_node *node)
{
    assert(node       != nullptr);
    assert(node->type == NODE_OP);

    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    /*
    log_message(__PRETTY_FUNCTION__);
    log_message("\nnode->type = %d\n", (node)->type);
    if ((node)->type == NODE_OP)
    {
        log_message("node->op_type = %d\n\n", (node)->value.op);
    }
    else log_message("\n");
    */
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

    if (node->left ->type == NODE_NUM &&
        node->right->type == NODE_NUM)
    {
        double val_left  = node->left ->value.dbl;
        double val_right = node->right->value.dbl;

        node_dtor(node->left );
        node_dtor(node->right);

        num_ctor(node, Tree_counter(val_left, val_right, node->value.op));
        return true;
    }
    return false;
}

static double Tree_counter(const double left, const double right, TYPE_OP op)
{
    switch (op)
    {
        case OP_ADD: return left + right;
        case OP_SUB: return left - right;
        case OP_MUL: return left * right;
        case OP_DIV: return left / right;
        case OP_SIN: return sin   (right);
        case OP_COS: return cos   (right);

        default    : log_error      ("default case in Tree_counter() op-switch: op = %d.\n", op);
                     assert(false && "default case in Tree_counter() op-switch");
                     break;
    }
    return 0;
}

/*_____________________________________________________________________*/

static bool Tree_optimize_add_main(Tree_node **node)
{
    assert(  node        != nullptr);
    assert( *node        != nullptr);
    assert((*node)->type == NODE_OP);

    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    /*
    log_message(__PRETTY_FUNCTION__);
    log_message("\nnode->type = %d\n", (*node)->type);
    if ((*node)->type == NODE_OP)
    {
        log_message("node->op_type = %d\n\n", (*node)->value.op);
    }
    else log_message("\n");
    */
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

    if ((*node)->value.op != OP_ADD) return false;

    if ((*node)->left ->type == NODE_NUM && approx_equal(0, (*node)->left ->value.dbl))
    {
        Tree_optimize_add_sub_mul_div(node, &((*node)->left ), &((*node)->right));
        return true;
    }
    if ((*node)->right->type == NODE_NUM && approx_equal(0, (*node)->right->value.dbl))
    {
        Tree_optimize_add_sub_mul_div(node, &((*node)->right), &((*node)->left ));
        return true;
    }

    return false;
}

static bool Tree_optimize_sub_main(Tree_node **node)
{
    assert(  node        != nullptr);
    assert( *node        != nullptr);
    assert((*node)->type == NODE_OP);

    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    /*
    log_message(__PRETTY_FUNCTION__);
    log_message("\nnode->type = %d\n", (*node)->type);
    if ((*node)->type == NODE_OP)
    {
        log_message("node->op_type = %d\n\n", (*node)->value.op);
    }
    else log_message("\n");
    */
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

    if ((*node)->value.op != OP_SUB) return false;

    if ((*node)->right->type == NODE_NUM && approx_equal(0, (*node)->right->value.dbl))
    {
        Tree_optimize_add_sub_mul_div(node, &((*node)->right), &((*node)->left));
        return true;
    }
    return false;
}

static bool Tree_optimize_mul_main(Tree_node **node)
{
    assert( node         != nullptr);
    assert(*node         != nullptr);
    assert((*node)->type == NODE_OP);

    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    /*
    log_message(__PRETTY_FUNCTION__);
    log_message("\nnode->type = %d\n", (*node)->type);
    if ((*node)->type == NODE_OP)
    {
        log_message("node->op_type = %d\n\n", (*node)->value.op);
    }
    else log_message("\n");
    */
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

    if ((*node)->value.op != OP_MUL) return false;

    if (((*node)->right->type == NODE_NUM && approx_equal(0, (*node)->right->value.dbl)) ||
        ((*node)->left ->type == NODE_NUM && approx_equal(0, (*node)->left ->value.dbl)))
    {
        Tree_dtor((*node)->left );
        Tree_dtor((*node)->right);

        num_ctor(*node, 0);
        return true;
    }

    if ((*node)->right->type == NODE_NUM && approx_equal(1, (*node)->right->value.dbl))
    {
        Tree_optimize_add_sub_mul_div(node, &((*node)->right), &((*node)->left));
        return true;
    }
    if ((*node)->left ->type == NODE_NUM && approx_equal(1, (*node)->left-> value.dbl))
    {
        Tree_optimize_add_sub_mul_div(node, &((*node)->left ), &((*node)->right));
        return true;
    }
    return false;
}

static bool Tree_optimize_div_main(Tree_node **node)
{
    assert(  node        != nullptr);
    assert( *node        != nullptr);
    assert((*node)->type == NODE_OP);

    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    /*
    log_message(__PRETTY_FUNCTION__);
    log_message("\nnode->type = %d\n", (*node)->type);
    if ((*node)->type == NODE_OP)
    {
        log_message("node->op_type = %d\n\n", (*node)->value.op);
    }
    else log_message("\n");
    */
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

    if ((*node)->value.op != OP_DIV) return false;

    if ((*node)->left->type == NODE_NUM && approx_equal(0, (*node)->left->value.dbl))
    {
        Tree_dtor((*node)->left );
        Tree_dtor((*node)->right);

        num_ctor(*node, 0);
        return true;
    }

    if ((*node)->right->type == NODE_NUM && approx_equal(1, (*node)->right->value.dbl))
    {
        Tree_optimize_add_sub_mul_div(node, &((*node)->right), &((*node)->left));
        return true;
    }
    return false;
}

/*_____________________________________________________________________*/

static void Tree_optimize_add_sub_mul_div(Tree_node **node, Tree_node **null_son, Tree_node **good_son)
{
    assert( node     != nullptr);
    assert(*node     != nullptr);
    assert( null_son != nullptr);
    assert(*null_son != nullptr);
    assert( good_son != nullptr);
    assert(*good_son != nullptr);

    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    /*
    log_message(__PRETTY_FUNCTION__);
    
    log_message("\nnode: %p\n", *node);
    log_message("null: %p\n", *null_son);
    log_message("good: %p\n", *good_son);
    log_message("(*node)->prev: %p\n", (*node)->prev);

    log_message("\nnode->op_type  = %d \n", (*node)->value.op);
    log_message("null_son->num  = %lg\n", (*null_son)->value.dbl);
    log_message("good_son->type = %d \n", (*good_son)->type);
    
    Tree_dump_graphviz((*node)->prev);
    */
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

    node_dtor(*null_son);

    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    /*
    log_message("  node       : %p\n",  node);
    log_message("(*node)      : %p\n", *node);
    log_message("(*node)->prev: %p\n\n", (*node)->prev);
    */
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

    if ((*node)->prev == nullptr) // change the root of the tree
    {
        (*node) = *good_son;
        node_dtor((*node)->prev);
        (*node)->prev = nullptr;

        return;
    }

    //create copies to avoid loss of old values

    Tree_node *link_node     = &(**node);
    Tree_node *link_null_son = &(**null_son);
    Tree_node *link_good_son = &(**good_son);

    node     = &link_node;
    null_son = &link_null_son;
    good_son = &link_good_son;
    
    //delete the "node" from the tree and rehang the "good_son"

    if ((*node)->prev->left == (*node))
    {
        (*node    )->prev->left  =  *good_son;
        (*good_son)->prev        = (*node    )->prev;
    }
    else
    {
        //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
        /*
        log_message("else case\n");
        log_message("%p\n", &((*node)->prev->right));
        log_message("%p\n\n", &(*(**node).prev).right);
        */
        //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

        (*node)    ->prev->right =  *good_son;
        
        //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
        /*
        log_message("  node       : %p\n",  node);
        log_message("(*node)      : %p\n", *node);
        log_message("(*node)->prev: %p\n\n", (*node)->prev);
        */
        //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
        
        (*good_son)->prev        = (*node    )->prev;
        
        //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
        /*
        log_message("  node       : %p\n",  node);
        log_message("(*node)      : %p\n", *node);
        log_message("(*node)->prev: %p\n\n", (*node)->prev);
        */
        //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    }

    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    /*
    log_message("(*node)->prev: %p\n", (*node)->prev);
    Tree_dump_graphviz((*node)->prev);
    */
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    
    node_dtor(*node);
}

/*_____________________________________________________________________*/


Tree_node *diff_main(Tree_node **root)
{
    log_header(__PRETTY_FUNCTION__);

    if (root == nullptr)
    {
        log_error     ("Nullptr-pointer to the tree to differentiate.\n");
        log_end_header();
        return nullptr;
    }
    if (Tree_verify(*root) == false)
    {
        log_error     ("Can't differentiate the function, because tree is invalid.\n");
        log_end_header();
        return nullptr;
    }
    Tree_optimize_main(root);
    Tree_node     *diff_root = diff_execute(*root);
    diff_prev_init(diff_root, nullptr);

    return diff_root;
}

static Tree_node *diff_execute(Tree_node *const node)
{
    assert(node != nullptr);

    switch(node->type)
    {
        case NODE_NUM: return new_node_num(0, nullptr);
        
        case NODE_VAR: return new_node_num(1, nullptr);
        
        case NODE_OP:  switch(node->value.op)
                       {
                           case OP_ADD: return Add(dL(node), dR(node));

                           case OP_SUB: return Sub(dL(node), dR(node));

                           case OP_MUL: return Add(Mul(dL(node), cR(node)), Mul(cL(node), dR(node)));

                           case OP_DIV: return Div(Sub(Mul(dL(node), cR(node)), Mul(cL(node), dR(node))), Mul(cR(node), cR(node)));
                           
                           case OP_SIN: return Mul(Cos(cL(node), cR(node)), dR(node));

                           case OP_COS: return Mul(Sub(cL(node), Sin(cL(node), cR(node))), dR(node));
                           
                           case OP_POW: 
                           default    : log_error      ("default case in diff_execute() in TYPE-OP-switch: op_type = %d.\n", node->value.op);
                                        Tree_dump_graphviz(node);
                                        assert(false && "default case in diff_execute() in TYPE_OP-switch");
                                        return nullptr;
                       }
                       return nullptr;
        
        case NODE_UNDEF:
        default        : log_error      ("default case in diff_execute() in TYPE-NODE-switch: node_type = %d.\n", node->type);
                         Tree_dump_graphviz(node);
                         assert(false && "default case in TYPE_NODE-switch");
                         return nullptr;
    }

    return nullptr;
}
/*
static Tree_node *diff_op_pow(Tree_node *const node)
{
    assert(node           != nullptr);
    assert(node->type     == NODE_OP);
    assert(node->value.op ==  OP_POW);
}
*/
static void diff_prev_init(Tree_node *diff_node, Tree_node *prev)
{
    assert(diff_node);

    diff_node->prev = prev;

    if (diff_node->left ) diff_prev_init(diff_node->left , diff_node);
    if (diff_node->right) diff_prev_init(diff_node->right, diff_node);
}

static Tree_node *Tree_copy(Tree_node *cp_from)
{
    assert(cp_from != nullptr);

    switch (cp_from->type)
    {
        case NODE_NUM   : return new_node_num(cp_from->value.dbl, nullptr);

        case NODE_VAR   : return new_node_var(nullptr);

        case NODE_OP    : return new_node_op(cL(cp_from), cR(cp_from), nullptr, cp_from->value.op);

        case NODE_UNDEF :
        default         : log_error      ("default case in Tree_copy() in TYPE-NODE-switch: node_type = %d.\n", cp_from->type);
                          Tree_dump_graphviz(cp_from);
                          assert(false && "default case in Tree_copy() in TYPE-NODE-switch");
                          break;
    }
    return nullptr;
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

    char    dump_txt[GRAPHVIZ_SIZE_FILE] = "";
    char    dump_png[GRAPHVIZ_SIZE_FILE] = "";

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

    char cmd[GRAPHVIZ_SIZE_CMD] = "";
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
                        sprintf(value, "op: %s", op_names[node->value.op]);
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

    if      (node->type == NODE_NUM     ) log_message("%lg", node->value.dbl);
    else if (node->type == NODE_VAR     ) log_message("x");
    else if (node->value.op == OP_SIN ||
             node->value.op == OP_COS   ) dump_txt_sin_cos(node, bracket);
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

static void dump_txt_sin_cos(Tree_node *node, bool bracket)
{
    assert(node->value.op == OP_SIN ||
           node->value.op == OP_COS   );

    if (bracket) log_message("(");

    log_message(op_names[node->value.op]);
    Tree_dump_txt_dfs(node->right, true);

    if (bracket) log_message(")");
}

/*_____________________________________________________________________*/

void Tree_dump_tex(Tree_node *root)
{
    log_header(__PRETTY_FUNCTION__);

    if (Tree_verify(root) == false)
    {
        log_message("Can't generate tex_dump, because tree is invalid.\n"
                    "Get graphviz_dump.\n");
        Tree_dump_graphviz(root);
        return;
    }

    static int cur = 0;

    char    dump_tex[FILE_SIZE] = "";
    char    dump_pdf[FILE_SIZE] = "";

    sprintf(dump_tex, "dump_tex/Tree%d.tex", cur);
    sprintf(dump_pdf, "dump_pdf/Tree%d.pdf", cur);

    FILE *stream_txt =  fopen(dump_tex, "w");
    if   (stream_txt == nullptr)
    {
        log_error     ("Can't open tex-dump file.\n");
        log_end_header();
        return;
    }

    setvbuf(stream_txt, nullptr, _IONBF, 0);
    fprintf(stream_txt, tex_header);
    fprintf(stream_txt, "$$\n");

    Tree_dump_tex_dfs(root, false, stream_txt);

    fprintf(stream_txt, "\n$$\n");
    fprintf(stream_txt, "\\end{document}\n");
    
    Tree_dump_tex_system(dump_tex, dump_pdf, cur); ++cur;
    log_message         ("<object><embed src=\"%s\" width=\"%d\" height=\"%d\"/></object>\n", dump_pdf, PDF_WIDTH, PDF_HEIGHT);
    log_end_header      ();

    fclose(stream_txt);
}

static void Tree_dump_tex_system(char *const dump_tex, char *const dump_pdf, const int cur)
{
    assert(dump_tex);
    assert(dump_pdf);

    char cmd[CMD_SIZE] = "";
    sprintf       (cmd, "pdflatex %s", dump_tex);
    system        (cmd);

    sprintf       (dump_tex, "Tree%d", cur);
    sprintf       (cmd, "rm %s.log", dump_tex);
    system        (cmd);

    sprintf       (cmd, "rm %s.aux", dump_tex);
    system        (cmd);

    sprintf       (cmd, "cp %s.pdf %s", dump_tex, dump_pdf);
    system        (cmd);

    sprintf       (cmd, "rm %s.pdf", dump_tex);
    system        (cmd);
}

static void Tree_dump_tex_dfs(Tree_node *node, bool bracket, FILE *const stream)
{
    assert(node   != nullptr);
    assert(stream != nullptr);

    if (bracket) fprintf(stream, "(");

    if (node->type     == NODE_NUM  )
    {
        if (node->value.dbl >= 0) fprintf(stream, "%lg"  , node->value.dbl);
        else                      fprintf(stream, "(%lg)", node->value.dbl);
    }
    else if (node->type     == NODE_VAR  ) fprintf(stream, " x");
    else if (node->value.op ==   OP_SUB && Tree_dump_tex_op_sub(node, stream)) ;
    else if (node->value.op ==   OP_DIV  ) Tree_dump_tex_op_div(node, stream);
    else if (node->value.op ==   OP_SIN ||
             node->value.op ==   OP_COS  ) Tree_dump_tex_op_sin_cos(node, stream);
    else
    {
        if (node->left->type != NODE_OP || op_priority[node->left->value.op] >= op_priority[node->value.op])
        {
            Tree_dump_tex_dfs(node->left, false, stream);
        }
        else { Tree_dump_tex_dfs(node->left, true, stream); }

        fprintf(stream, op_names[node->value.op]);

        if (node->right->type != NODE_OP || op_priority[node->right->value.op] > op_priority[node->value.op])
        {
            Tree_dump_tex_dfs(node->right, false, stream);
        }
        else { Tree_dump_tex_dfs(node->right, true, stream); }
    }

    if (bracket) fprintf(stream, ")");
}

static void Tree_dump_tex_op_sin_cos(Tree_node *node, FILE *const stream)
{
    assert(node           != nullptr);
    assert(stream         != nullptr);
    assert(node->type     == NODE_OP);
    assert(node->value.op == OP_SIN ||
           node->value.op == OP_COS );

    fprintf          (stream, " %s", op_names[node->value.op]);
    Tree_dump_tex_dfs(node->right, true, stream);
}

static void Tree_dump_tex_op_div(Tree_node *node, FILE *const stream)
{
    assert(node           != nullptr);
    assert(stream         != nullptr);
    assert(node->type     == NODE_OP);
    assert(node->value.op == OP_DIV );

    fprintf          (stream, "\\frac{");
    Tree_dump_tex_dfs(node->left , false, stream);
    fprintf          (stream, "}{");
    Tree_dump_tex_dfs(node->right, false, stream);
    fprintf          (stream, "}");
}

static bool Tree_dump_tex_op_sub(Tree_node *node, FILE *const stream)
{
    assert(node           != nullptr);
    assert(stream         != nullptr);
    assert(node->type     == NODE_OP);
    assert(node->value.op == OP_SUB );

    if (node->left->type == NODE_NUM && approx_equal(0, node->left->value.op))
    {
        fprintf(stream, "-");
        Tree_dump_tex_dfs(node->right, false, stream);
        return true;
    }
    return false;
}
/*_____________________________________________________________________*/