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
static bool         Tree_vars_verify        (                           Tree_node *const node);
static void         print_error_messages    (unsigned int        err);

static void         dfs_dtor                (Tree_node *const node);

static bool         Tree_parsing_execute    (Tree_node *const root, const char *data     ,
                                                                    const int   data_size,
                                                                    int *const  data_pos );
static bool         is_char_var             (const char c);
static VAR          get_var_from_char       (const char c);

static bool         push_next_node          (Tree_node **node, Tree_node *const root);
static bool         push_prev_node          (Tree_node **node, Tree_node *const root);

static bool         put_dbl                 (Tree_node *const node, bool e_val, const char *data    ,
                                                                                int *const  data_pos);
static bool         get_dbl                 (double *const dbl,     const char *data    ,
                                                                    int *const  data_pos);

static bool         put_var                 (Tree_node *const node, const char possible_var);
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
static bool         Tree_optimize_pow_main          (Tree_node **node);
static void         Tree_optimize_all               (Tree_node **node, Tree_node **null_son, Tree_node **good_son);
static void         Tree_vars_update                (Tree_node *const node);

static Tree_node   *diff_execute            (Tree_node *const node, VAR var);
static Tree_node   *diff_op_var             (Tree_node *const node, VAR var);
static Tree_node   *diff_op_case            (Tree_node *const node, VAR var);
static Tree_node   *diff_op_pow             (Tree_node *const node, VAR var);
static void         diff_prev_init          (Tree_node *diff_node, Tree_node *prev);
static Tree_node   *Tree_copy               (Tree_node *cp_from);

static double       dfs_value_in_point      (const Tree_node *node, const double x_val);

static void         Tree_dump_graphviz_dfs  (Tree_node *node, int *const node_number, FILE *const stream);
static void         Tree_node_describe      (Tree_node *node, int *const node_number, FILE *const stream);
static void         print_Tree_node         (Tree_node *node, int *const node_number, FILE *const stream,   GRAPHVIZ_COLOR fillcolor,
                                                                                                            GRAPHVIZ_COLOR     color, 
                                                                                                            const char    *node_type,
                                                                                                            const char        *value);
static void         Tree_dump_txt_dfs           (Tree_node *node, bool bracket);
static void         dump_txt_num                (Tree_node *node);
static void         dump_txt_unary              (Tree_node *node);

static void         Tree_dump_tex_system        (char *const dump_tex, char *const dump_pdf, const int cur);

static void         Tree_dump_tex_dfs           (Tree_node *node, bool bracket, FILE *const stream);
static void         Tree_dump_tex_dfs_with_value(Tree_node *node, bool bracket, FILE *const stream, const double x_val);

static void         Tree_dump_tex_op_unary      (Tree_node *node, FILE *const stream);
static void         Tree_dump_tex_op_sqrt       (Tree_node *node, FILE *const stream);
static void         Tree_dump_tex_op_div        (Tree_node *node, FILE *const stream);
static void         Tree_dump_tex_op_pow        (Tree_node *node, FILE *const stream);
static bool         Tree_dump_tex_op_sub        (Tree_node *node, FILE *const stream);
static void         dump_tex_num                (Tree_node *node, FILE *const stream);
static void         dump_tex_num                (const double num,FILE *const stream);

static void         Tex_tree                    (Tree_node *root, FILE *const stream,   const char *text_before,
                                                                                        const char *text_after , bool is_val, const double x_val);

/*___________________________STATIC_CONST______________________________*/

static const Tree_node default_node = 
{
    NODE_UNDEF  , // TYPE_NODE

    nullptr     , // left
    nullptr     , // right
    nullptr     , // prev

          0     , // subtree_vars

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
    4   , // OP_TAN
    3   , // OP_POW
    4   , // OP_LOG
    4   , // OP_SQRT
    4   , // OP_SH
    4   , // OP_CH
    4   , // OP_ASIN
    4   , // OP_ACOS
    4   , // OP_ATAN
};

static const char *op_names[] =
{
    "+"     , // OP_ADD
    "-"     , // OP_SUB
    "*"     , // OP_MUL
    "/"     , // OP_DIV
    "sin"   , // OP_SIN
    "cos"   , // OP_COS
    "tg"    , // OP_TAN
    "^"     , // OP_POW
    "ln"    , // OP_LOG
    "sqrt"  , // OP_SQRT
    "sh"    , // OP_SH
    "ch"    , // OP_CH
    "arcsin", // OP_ASIN
    "arccos", // OP_ACOS
    "arctan", // OP_ATAN
};

static const char *var_names[] =
{
    "x"         ,
    "y"         ,
    "z"         ,
    "\\alpha"   ,
    "\\beta"    ,
    "\\gamma"   ,
    "\\lambda"  ,
    "\\mu"      ,
};

static const int VALUE_SIZE = 100;
static const int  FILE_SIZE = 100;
static const int   CMD_SIZE = 300;
static const int PDF_WIDTH  = 500;
static const int PDF_HEIGHT = 500;

static const double e       = exp(1);

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

    INVALID_TREE_VARS   ,
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
    
    "Invalid tree_vars-values in nodes.\n"  ,
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

//___________________

#define getL              l(node)
#define getR              r(node)
#define getP              p(node)
#define getOP            op(node)
#define getDBL          dbl(node)
#define getVAR          var(node)
#define TREE_VARS tree_vars(node)

//___________________

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

    if (getL) Tree_verify_dfs(err, root, getL);
    if (getR) Tree_verify_dfs(err, root, getR);

    bool is_terminal_node = false;

    if (getR == nullptr &&
        getL == nullptr   ) is_terminal_node = true;

    if (getP == nullptr && node != root) (*err) = (*err) | (1 << NULLPTR_PREV );
    if (getL == nullptr && getR        ) (*err) = (*err) | (1 << NULLPTR_LEFT );
    if (getR == nullptr && getL        ) (*err) = (*err) | (1 << NULLPTR_RIGHT);

    if (node->type  == NODE_UNDEF)
    {
        (*err) = (*err) | (1 << UNDEF_TYPE_NODE);
        return;
    }

    if      (node->type == NODE_OP  &&  is_terminal_node) (*err) = (*err) | (1 << TERMINAL_OP     );
    else if (node->type == NODE_NUM && !is_terminal_node) (*err) = (*err) | (1 << NON_TERMINAL_NUM);
    else if (node->type == NODE_VAR && !is_terminal_node) (*err) = (*err) | (1 << NON_TERMINAL_VAR);

    if (*err == 0 && !Tree_vars_verify(node))             (*err) = (*err) | (1 << INVALID_TREE_VARS);
}

static bool Tree_vars_verify(Tree_node *const node)
{
    switch(node->type)
    {
        case NODE_NUM: return TREE_VARS ==           0;
        case NODE_VAR: return TREE_VARS == 1 << getVAR;
        case NODE_OP : return TREE_VARS == (tree_vars(getL) | tree_vars(getR));

        case NODE_UNDEF:
        default        : log_error(      "default case in Tree_vars_verify(): node-type = %d.\n", node->type);
                         assert(false && "default case in Tree_vars_verify()");
                         break;
    }
    return false;
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

void node_op_ctor(Tree_node *const node,    TYPE_OP             value,
                                            Tree_node *const     left,
                                            Tree_node *const    right,
                                            Tree_node *const     prev)
{
    node->type = NODE_OP;

    getL      =     left;
    getR      =    right;
    getP      =     prev;
    getOP     =    value;
    TREE_VARS =        0;

    if (getL != nullptr &&
        getR != nullptr   ) TREE_VARS = tree_vars(getL) | tree_vars(getR);
}

void node_num_ctor(Tree_node *const node,   const double    value,
                                            Tree_node *const prev)
{
    assert(node != nullptr);

    node->type = NODE_NUM;
    
    getL      =  nullptr;
    getR      =  nullptr;
    getP      =     prev;
    TREE_VARS =        0;
    getDBL    =    value;
}

void node_var_ctor(Tree_node *const node,   VAR             value,
                                            Tree_node *const prev)
{
    assert(node != nullptr);

    node->type = NODE_VAR;
    
    getL      =    nullptr;
    getR      =    nullptr;
    getP      =       prev;
    TREE_VARS = 1 << value;
    getVAR    =      value;
}

void node_undef_ctor(Tree_node *const node, Tree_node *const prev)
{
    assert (node != nullptr);

    *node  = default_node;
    getP      =      prev;
}

//___________________

#undef getL
#undef getR
#undef getP
#undef getOP
#undef getDBL
#undef getVAR

//___________________

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

    node_op_ctor(new_node, value, new_left, new_right, prev);
    return       new_node;
}

Tree_node *new_node_op(TYPE_OP    value,
                       Tree_node *left ,
                       Tree_node *right,
                       Tree_node *prev )
{
    assert(left  != nullptr);
    assert(right != nullptr);

    Tree_node *new_node = (Tree_node *) log_calloc(1, sizeof(Tree_node));
    if        (new_node == nullptr)
    {
        log_message("log_calloc returns nullptr in %s.\n", __PRETTY_FUNCTION__);
        return nullptr;
    }

    node_op_ctor(new_node, value, left, right, prev);
    return       new_node;
}

Tree_node *new_node_num(const double value, Tree_node *const prev)
{
    Tree_node *new_node = (Tree_node *) log_calloc(1, sizeof(Tree_node));
    if        (new_node == nullptr) return nullptr;

    node_num_ctor(new_node, value, prev);
    return        new_node;
}

Tree_node *new_node_var(VAR value, Tree_node *const prev)
{
    Tree_node *new_node = (Tree_node *) log_calloc(1, sizeof(Tree_node));
    if        (new_node == nullptr) return nullptr;

    node_var_ctor(new_node, value, prev);
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

//___________________

#define getL     l(node)
#define getR     r(node)
#define getP     p(node)
#define getOP   op(node)
#define getDBL dbl(node)
#define getVAR var(node)

//___________________

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

    if (getL != nullptr) dfs_dtor(getL);
    if (getR != nullptr) dfs_dtor(getR);

    node_dtor(node);
}

//___________________

#undef getL
#undef getR
#undef getP
#undef getOP
#undef getDBL
#undef getVAR

//___________________

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

    if (!Tree_parsing_execute(root, data, data_size, &data_pos))
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

        if      (cur_char == '('      ) { if (!push_next_node(&cur_node, root             )) return false; }
        else if (cur_char == ')'      ) { if (!push_prev_node(&cur_node, root             )) return false; }
        else if (isdigit(cur_char)    ) { if (!put_dbl       ( cur_node, false, data      ,
                                                                                data_pos  )) return false; }
        else if (cur_char == 'e'      ) { if (!put_dbl       ( cur_node, true , data      ,
                                                                                data_pos  )) return false; }
        else if (is_char_var(cur_char)) { if (!put_var       ( cur_node, cur_char         )) return false; }
        else                            { if (!put_op        ( cur_node,        data      ,
                                                                                data_size ,
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

static bool is_char_var(const char c)
{
    return c == 'x' || c == 'y' || c == 'z';
}

static VAR get_var_from_char(const char c)
{
    switch (c)
    {
        case 'x': return X;
        case 'y': return Y;
        case 'z': return Z;
        default : assert(false && "default case in get_var_from_char()");
    }
    return X;
}

//___________________

#define getL     l(*node)
#define getR     r(*node)
#define getP     p(*node)
#define getOP   op(*node)
#define getDBL dbl(*node)
#define getVAR var(*node)

//___________________

static bool push_next_node(Tree_node **node, Tree_node *const root)
{
    assert(root != nullptr);
    assert(node != nullptr);

    if (*node == nullptr)
    {
       *node = root;
        return true;
    }

    if (getL != nullptr)
    {
        if (getR->type == NODE_UNDEF)
        {
            *node = getR;
            return true;
        }
        else
        {
            log_error("Pushing in the third time.\n");
            return false;
        }
    }

    getL = new_node_undef(*node);
    getR = new_node_undef(*node);
    
    if (getL == nullptr ||
        getR == nullptr)
    {
        log_free(getL);
        log_free(getR);

        log_error("Can't create new node.\n");
        return false;
    }

    *node = getL;
    return  true;
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

   *node = getP;
   return true;
}

//___________________

#undef getL
#undef getR
#undef getP
#undef getOP
#undef getDBL
#undef getVAR

#define getL     l(node)
#define getR     r(node)
#define getP     p(node)
#define getOP   op(node)
#define getDBL dbl(node)
#define getVAR var(node)

//___________________

static bool put_dbl(Tree_node *const node,  bool e_val, const char *data    ,
                                                        int *const  data_pos)
{
    assert(data     != nullptr);
    assert(data_pos != nullptr);
    if    (node     == nullptr)
    {
        log_error("Can't put double-value in nullptr-node.\n");
        return false;
    }
    if (getL != nullptr ||
        getR != nullptr)
    {
        log_error("Can't put double-number in non-terminal node.\n");
        return false;
    }
    if (node->type != NODE_UNDEF)
    {
        log_error("Redefinition of dbl-node.\n");
        return false;
    }
    
    if (e_val)
    {
        num_ctor(node, e);
        return true;
    }

    double dbl =             0;
   *data_pos   = *data_pos - 1;
    
    if (get_dbl(&dbl, data, data_pos))
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

static bool put_var(Tree_node *const node, const char possible_char)
{
    if (node == nullptr)
    {
        log_error("Can't put variable in nullptr-node.\n");
        return false;
    }
    if (getL != nullptr ||
        getR != nullptr)
    {
        log_error("Can't put variable in non-terminal mode");
        return false;
    }
    if (node->type != NODE_UNDEF)
    {
        log_error("Redefinition of var-node.\n");
        return false;
    }

    var_ctor(node, get_var_from_char(possible_char));
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
    if (getL == nullptr ||
        getR == nullptr)
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
    get_word_split(possible_op, VALUE_SIZE, data, data_size, data_pos, "0123456789()xe");

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

//___________________

#undef getL
#undef getR
#undef getP
#undef getOP
#undef getDBL
#undef getVAR

//___________________

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

    Tree_optimize_execute( root);
    Tree_vars_update     (*root);
    Tree_verify          (*root);
    log_end_header       (     );
}

static void Tree_optimize_execute(Tree_node **node)
{
    assert( node != nullptr);
    assert(*node != nullptr);

    if ((*node)->type != NODE_OP) return;

    Tree_optimize_execute(&(l(*node)));
    Tree_optimize_execute(&(r(*node)));

    if (Tree_optimize_numbers(*node)) return;
    if (Tree_optimize_add_main(node)) return;
    if (Tree_optimize_sub_main(node)) return;
    if (Tree_optimize_mul_main(node)) return;
    if (Tree_optimize_div_main(node)) return;
    if (Tree_optimize_pow_main(node)) return;
}

/*_____________________________________________________________________*/

//___________________

#define getL     l(node)
#define getR     r(node)
#define getP     p(node)
#define getOP   op(node)
#define getDBL dbl(node)
#define getVAR var(node)

//___________________

static bool Tree_optimize_numbers(Tree_node *node)
{
    assert(node       != nullptr);
    assert(node->type == NODE_OP);

    if (getL->type == NODE_NUM &&
        getR->type == NODE_NUM)
    {
        double val_left   = dbl(getL);
        double val_right  = dbl(getR);
        double val_result = Tree_counter(val_left, val_right, getOP);

        node_dtor(getL);
        node_dtor(getR);

        num_ctor(node, val_result);
        return true;
    }
    return false;
}

//___________________

#undef getL
#undef getR
#undef getP
#undef getOP
#undef getDBL
#undef getVAR

//___________________

static double Tree_counter(const double left, const double right, TYPE_OP op)
{
    switch (op)
    {
        case OP_ADD : return left + right;
        case OP_SUB : return left - right;
        case OP_MUL : return left * right;
        case OP_DIV : return left / right;
        case OP_SIN : return sin   (right);
        case OP_COS : return cos   (right);
        case OP_TAN : return tan   (right);
        case OP_POW : return pow   (left, right);
        case OP_LOG : return log   (right);
        case OP_SQRT: return sqrt  (right);
        case OP_SH  : return sinh  (right);
        case OP_CH  : return cosh  (right);
        case OP_ASIN: return asin  (right);
        case OP_ACOS: return acos  (right);
        case OP_ATAN: return atan  (right);

        default     : log_error      ("default case in Tree_counter() op-switch: op = %d.\n", op);
                      assert(false && "default case in Tree_counter() op-switch");
                      break;
    }
    return 0;
}

/*_____________________________________________________________________*/

//___________________

#define getL     l(*node)
#define getR     r(*node)
#define getP     p(*node)
#define getOP   op(*node)
#define getDBL dbl(*node)
#define getVAR var(*node)

//___________________

static bool Tree_optimize_add_main(Tree_node **node)
{
    assert(  node        != nullptr);
    assert( *node        != nullptr);
    assert((*node)->type == NODE_OP);

    if (getOP != OP_ADD) return false;

    if (getL->type == NODE_NUM && approx_equal(0, dbl(getL)))
    {
        Tree_optimize_all(node, &(getL), &(getR));
        return true;
    }
    if (getR->type == NODE_NUM && approx_equal(0, dbl(getR)))
    {
        Tree_optimize_all(node, &(getR), &(getL));
        return true;
    }

    return false;
}

static bool Tree_optimize_sub_main(Tree_node **node)
{
    assert(  node        != nullptr);
    assert( *node        != nullptr);
    assert((*node)->type == NODE_OP);

    if (getOP != OP_SUB) return false;

    if (getR->type == NODE_NUM && approx_equal(0, dbl(getR)))
    {
        Tree_optimize_all(node, &(getR), &(getL));
        return true;
    }
    return false;
}

static bool Tree_optimize_mul_main(Tree_node **node)
{
    assert(  node        != nullptr);
    assert( *node        != nullptr);
    assert((*node)->type == NODE_OP);

    if (getOP != OP_MUL) return false;

    if ((getR->type == NODE_NUM && approx_equal(0, dbl(getR))) ||
        (getL->type == NODE_NUM && approx_equal(0, dbl(getL))))
    {
        Tree_dtor(getL);
        Tree_dtor(getR);

        num_ctor(*node, 0);
        return true;
    }

    if (getR->type == NODE_NUM && approx_equal(1, dbl(getR)))
    {
        Tree_optimize_all(node, &(getR), &(getL));
        return true;
    }
    if (getL->type == NODE_NUM && approx_equal(1, dbl(getL)))
    {
        Tree_optimize_all(node, &(getL), &(getR));
        return true;
    }
    return false;
}

static bool Tree_optimize_div_main(Tree_node **node)
{
    assert(  node        != nullptr);
    assert( *node        != nullptr);
    assert((*node)->type == NODE_OP);

    if (getOP != OP_DIV) return false;

    if (getL->type == NODE_NUM && approx_equal(0, dbl(getL)))
    {
        Tree_dtor(getL);
        Tree_dtor(getR);

        num_ctor(*node, 0);
        return true;
    }

    if (getR->type == NODE_NUM && approx_equal(1, dbl(getR)))
    {
        Tree_optimize_all(node, &(getR), &(getL));
        return true;
    }
    return false;
}

static bool Tree_optimize_pow_main(Tree_node **node)
{
    assert(  node        != nullptr);
    assert( *node        != nullptr);
    assert((*node)->type == NODE_OP);

    if (getOP != OP_POW) return false;

    if (getL->type == NODE_NUM && approx_equal(1, dbl(getL)))
    {
        Tree_dtor(getL);
        Tree_dtor(getR);

        num_ctor(*node, 1);
        return true;
    }

    if (getR->type == NODE_NUM && approx_equal(0, dbl(getR)))
    {
        Tree_dtor(getL);
        Tree_dtor(getR);

        num_ctor(*node, 1);
        return true;
    }

    if (getR->type == NODE_NUM && approx_equal(1, dbl(getR)))
    {
        Tree_optimize_all(node, &(getR), &(getL));
        return true;
    }
    return false;
}

/*_____________________________________________________________________*/

static void Tree_optimize_all(Tree_node **node, Tree_node **null_son, Tree_node **good_son)
{
    assert( node     != nullptr);
    assert(*node     != nullptr);
    assert( null_son != nullptr);
    assert(*null_son != nullptr);
    assert( good_son != nullptr);
    assert(*good_son != nullptr);

    node_dtor(*null_son);

    if (getP == nullptr) // change the root of the tree
    {
        (*node) = *good_son;
        node_dtor(getP);
        getP = nullptr;

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

    if (getP->left == (*node))
    {
        l(p(*node   )) =   *good_son;
        p  (*good_son) = p(*node   );
    }
    else
    {
        r(p(*node   )) =   *good_son;
        p  (*good_son) = p(*node   );
    }
    
    node_dtor(*node);
}

static void Tree_vars_update(Tree_node *const node)
{
    assert(node != nullptr);

    if (l(node) != nullptr &&
        r(node) != nullptr   )
    {
        Tree_vars_update(l(node));
        Tree_vars_update(r(node));

        tree_vars(node) = tree_vars(l(node)) | tree_vars(r(node));
    }
}

//___________________

#undef getL
#undef getR
#undef getP
#undef getOP
#undef getDBL
#undef getVAR

//___________________

/*_____________________________________________________________________*/


Tree_node *diff_main(Tree_node **root, const char *vars)
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
    if (!(strlen(vars) == 1 && (is_char_var(vars[0]) || vars[0] == 'a')))
    {
        log_error     ("Undefined value of vars: \"%s\".\n", vars);
        log_end_header();
        return nullptr;
    }

    Tree_optimize_main(root);
    Tree_node *diff_root = nullptr;

    switch (vars[0])
    {
        case 'x': diff_root = diff_execute(*root, X);
                  break;
        case 'y': diff_root = diff_execute(*root, Y);
                  break;
        case 'z': diff_root = diff_execute(*root, Z);
                  break;
        default : diff_root = Add(diff_execute(*root, X), Add(diff_execute(*root, Y), diff_execute(*root, Z)));
                  break;
    }
    diff_prev_init(diff_root, nullptr);

    return diff_root;
}

//_____________________________

#define DL dL(node, var)
#define DR dR(node, var)
#define CL cL(node)
#define CR cR(node)

//_____________________________

static Tree_node *diff_execute(Tree_node *const node, VAR var)
{
    assert(node != nullptr);

    switch(node->type)
    {
        case NODE_NUM  : return Nul;
        
        case NODE_VAR  : return diff_op_var (node, var);
        
        case NODE_OP   : return diff_op_case(node, var);
        
        case NODE_UNDEF:
        default        : log_error      ("default case in diff_execute() in TYPE-NODE-switch: node_type = %d.\n", node->type);
                         Tree_dump_graphviz(node);
                         assert(false && "default case in TYPE_NODE-switch");
                         return nullptr;
    }

    return nullptr;
}

static Tree_node *diff_op_var(Tree_node *const node, VAR var)
{
    assert(node       != nullptr);
    assert(node->type == NODE_VAR);

    if (var(node) == var) return Num(1);
    return Num(0);
}

static Tree_node *diff_op_case(Tree_node *const node, VAR var)
{
    assert(node       != nullptr);
    assert(node->type == NODE_OP);

    switch(op(node))
    {
        case OP_ADD : return Add(DL, DR);
        case OP_SUB : return Sub(DL, DR);

        case OP_MUL : return Add(Mul(DL, CR), Mul(CL, DR));
        case OP_DIV : return Div(Sub(Mul(DL, CR), Mul(CL, DR)), Pow(CR, Num(2)));

        case OP_SIN : return Mul(Cos(CL, CR), DR);
        case OP_COS : return Mul(Sub(Nul, Sin(CL, CR)), DR);
        case OP_TAN : return Div(DR, Pow(Cos(CL, CR), Num(2)));

        case OP_LOG : return Div(DR, CR);

        case OP_POW : return diff_op_pow(node, var);

        case OP_SQRT: return Div(DR, Mul(Num(2), Sqrt(CL, CR)));

        case OP_SH  : return Mul(Ch(CL, CR), DR);
        case OP_CH  : return Mul(Sh(CL, CR), DR);

        case OP_ASIN: return         Div(DR, Sqrt(Nul, Sub(Num(1), Pow(CR, Num(2)))));
        case OP_ACOS: return Sub(0 , Div(DR, Sqrt(Nul, Sub(Num(1), Pow(CR, Num(2))))));
        case OP_ATAN: return Div(DR,                   Add(Num(1), Pow(CR, Num(2))));

        default     : log_error      ("default case in diff_execute() in TYPE-getOP-switch: op_type = %d.\n", op(node));
                      Tree_dump_graphviz(node);
                      assert(false && "default case in diff_execute() in TYPE_OP-switch");
                      return nullptr;
    }
    return nullptr;
}

static Tree_node *diff_op_pow(Tree_node *const node, VAR var)
{
    assert(node           != nullptr);
    assert(node->type     == NODE_OP);
    assert(op(node) ==  OP_POW);

    if (l(node)->type == NODE_NUM) return Mul(Pow(CL, CR), Mul(Log(Nul, CL), DR));
    if (r(node)->type == NODE_NUM) return Mul(Pow(CL, Num(dbl(r(node))-1)), Mul(CR, DL));
    
    return Mul(Pow(CL, CR), Add(Div(Mul(CR, DL), CL), Mul(DR, Log(Nul, CL))));
}

//_____________________________

#undef DL
#undef DR
#undef CL
#undef CR

//_____________________________

static void diff_prev_init(Tree_node *diff_node, Tree_node *prev)
{
    assert(diff_node);

    p(diff_node) = prev;

    if (l(diff_node)) diff_prev_init(l(diff_node), diff_node);
    if (r(diff_node)) diff_prev_init(r(diff_node), diff_node);
}

static Tree_node *Tree_copy(Tree_node *cp_from)
{
    assert(cp_from != nullptr);

    switch (cp_from->type)
    {
        case NODE_NUM   : return Num(dbl(cp_from));

        case NODE_VAR   : return new_node_var(var(cp_from));

        case NODE_OP    : return new_node_op (op(cp_from), cL(cp_from), cR(cp_from));

        case NODE_UNDEF :
        default         : log_error      ("default case in Tree_copy() in TYPE-NODE-switch: node_type = %d.\n", cp_from->type);
                          Tree_dump_graphviz(cp_from);
                          assert(false && "default case in Tree_copy() in TYPE-NODE-switch");
                          break;
    }
    return nullptr;
}

/*_____________________________________________________________________*/

//___________________

#define getL     l(node)
#define getR     r(node)
#define getP     p(node)
#define getOP   op(node)
#define getDBL dbl(node)
#define getVAR var(node)

//___________________

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
    if (getL)  Tree_dump_graphviz_dfs(getL, node_number, stream);

    int number_right = *node_number;
    if (getR) Tree_dump_graphviz_dfs(getR, node_number, stream);

    if (getL) fprintf(stream, "node%d->node%d[color=\"black\"]\n", number_cur, number_left );
    if (getR) fprintf(stream, "node%d->node%d[color=\"black\"]\n", number_cur, number_right);
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
                        sprintf(value, "var: %s", var_names[getVAR]);
                        break;
        
        case NODE_NUM:      color =  DARK_GREEN;
                        fillcolor = LIGHT_GREEN;
                        node_type =  "NODE_NUM";
                        sprintf(value, "dbl: %lg", getDBL);
                        break;

        case NODE_OP:       color =  DARK_ORANGE;
                        fillcolor = LIGHT_ORANGE;
                        node_type =    "NODE_OP";
                        sprintf(value, "op: %s", op_names[getOP]);
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
                                                    value);

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
                                                                                          getP,
                                                                                                         node_type,
                                                                                                                 value,
                                                                                                                              getL,
                                                                                                                                           getR);
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

    if      (node->type == NODE_NUM ) dump_txt_num(node);
    else if (node->type == NODE_VAR ) log_message ("%s", var_names[getVAR]);
    else
    {
        switch(getOP)
        {
            case OP_SIN :
            case OP_COS :
            case OP_TAN :
            case OP_SQRT:
            case OP_SH  :
            case OP_CH  :
            case OP_ASIN:
            case OP_ACOS:
            case OP_ATAN:
            case OP_LOG : dump_txt_unary(node);
                          break;
            
            default    : {
                            Tree_dump_txt_dfs(getL, !(getL->type != NODE_OP ||  op_priority[op(getL)] >= 
                                                                                op_priority[getOP]));
                            log_message(op_names[getOP]);

                            Tree_dump_txt_dfs(getR, !(getR->type != NODE_OP ||  op_priority[op(getR)] >
                                                                                op_priority[getOP]));
                            break;
                         }
        }
    }

    if (bracket) log_message(")");
}

static void dump_txt_num(Tree_node *node)
{
    assert(node       !=  nullptr);
    assert(node->type == NODE_NUM);

    if (approx_equal(e, dbl(node))) log_message("e");
    else if (dbl(node) >= 0)        log_message("%lg"  , dbl(node));
    else                            log_message("(%lg)", dbl(node));
}

static void dump_txt_unary(Tree_node *node)
{
    assert(node       != nullptr);
    assert(node->type == NODE_OP);

    log_message      (op_names[getOP]);
    Tree_dump_txt_dfs(getR, true);
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

    if      (node->type == NODE_NUM  ) dump_tex_num(node, stream);
    else if (node->type == NODE_VAR  ) fprintf(stream, " %s", var_names[getVAR]);
    else if (getOP      ==   OP_SUB && Tree_dump_tex_op_sub(node, stream));
    else
    {
        switch (getOP)
        {
            case OP_DIV: Tree_dump_tex_op_div(node, stream);
                         break;
            
            case OP_SIN :
            case OP_COS :
            case OP_TAN :
            case OP_SH  :
            case OP_CH  :
            case OP_ASIN:
            case OP_ACOS:
            case OP_ATAN:
            case OP_LOG : Tree_dump_tex_op_unary(node, stream);
                          break;
            
            case OP_SQRT: Tree_dump_tex_op_sqrt(node, stream);
                          break;

            case OP_POW : Tree_dump_tex_op_pow (node, stream);
                         break;

            default     : {
                            Tree_dump_tex_dfs(getL, !(getL->type != NODE_OP ||  op_priority[op(getL)] >= 
                                                                                op_priority[getOP]), stream);
                            fprintf(stream, op_names[getOP]);

                            Tree_dump_tex_dfs(getR, !(getR->type != NODE_OP ||  op_priority[op(getR)] >
                                                                                op_priority[getOP]), stream);
                            break;
                          }
        }
    }
    if (bracket) fprintf(stream, ")");
}

static void Tree_dump_tex_op_unary(Tree_node *node, FILE *const stream)
{
    assert(node       != nullptr);
    assert(stream     != nullptr);
    assert(node->type == NODE_OP);

    fprintf          (stream, " %s", op_names[getOP]);
    Tree_dump_tex_dfs(getR, true, stream);
}

static void Tree_dump_tex_op_sqrt(Tree_node *node, FILE *const stream)
{
    assert(node       != nullptr);
    assert(stream     != nullptr);
    assert(node->type == NODE_OP);
    assert(getOP      == OP_SQRT);

    fprintf          (stream, "\\sqrt{");
    Tree_dump_tex_dfs(getR, false, stream);
    fprintf          (stream, "}");
}

static void Tree_dump_tex_op_div(Tree_node *node, FILE *const stream)
{
    assert(node       != nullptr);
    assert(stream     != nullptr);
    assert(node->type == NODE_OP);
    assert(getOP      ==  OP_DIV);

    fprintf          (stream, "\\frac{");
    Tree_dump_tex_dfs(getL , false, stream);
    fprintf          (stream, "}{");
    Tree_dump_tex_dfs(getR, false, stream);
    fprintf          (stream, "}");
}

static void Tree_dump_tex_op_pow(Tree_node *node, FILE *const stream)
{
    assert(node       != nullptr);
    assert(stream     != nullptr);
    assert(node->type == NODE_OP);
    assert(getOP      ==  OP_POW);

    Tree_dump_tex_dfs(getL, false, stream);
    fprintf          (stream, "^{");
    Tree_dump_tex_dfs(getR, false, stream);
    fprintf          (stream, "}");
}

static bool Tree_dump_tex_op_sub(Tree_node *node, FILE *const stream)
{
    assert(node       != nullptr);
    assert(stream     != nullptr);
    assert(node->type == NODE_OP);
    assert(getOP      ==  OP_SUB);

    if (getL->type == NODE_NUM && approx_equal(0, dbl(getL)))
    {
        fprintf(stream, "-");
        Tree_dump_tex_dfs(getR, false, stream);
        return true;
    }
    return false;
}

static void dump_tex_num(Tree_node *node, FILE *const stream)
{
    assert(node       !=  nullptr);
    assert(node->type == NODE_NUM);

    dump_tex_num(dbl(node), stream);
}

static void dump_tex_num(const double num, FILE *const stream)
{
    assert(stream != nullptr);

    if (approx_equal(e, num)) fprintf(stream, "e");
    else if (num >= 0)        fprintf(stream, "%lg",   num);
    else                      fprintf(stream, "(%lg)", num);
}

/*_____________________________________________________________________*/

void Tex_head(const char *file, FILE **stream)
{
    log_header(__PRETTY_FUNCTION__);

    if (file == nullptr || stream == nullptr)
    {
        log_error("Pointer to the file or stream is nullptr.\n");
        return;
    }

    *stream = fopen(file, "w");
    if   (*stream == nullptr)
    {
        log_error     ("Can't open tex-dump file.\n");
        log_end_header();
        return;
    }

    setvbuf(*stream, nullptr, _IONBF, 0);
    fprintf(*stream, tex_header);

    log_end_header();
}

static void Tex_tree(Tree_node *root, FILE *const stream,   const char *text_before,
                                                            const char *text_after , bool is_val, const double x_val)
{
    log_header(__PRETTY_FUNCTION__);

    if (Tree_verify(root) == false)
    {
        log_end_header();
        log_error     ("Can't tex invalid tree.\n"
                       "Get graphviz_dump.\n");
        
        Tree_dump_graphviz(root);
        return;
    }

    if (text_before != nullptr) fprintf(stream, "%s", text_before);

    if (!is_val) Tree_dump_tex_dfs              (root, false, stream);
    else         Tree_dump_tex_dfs_with_value   (root, false, stream, x_val);

    if (text_after  != nullptr) fprintf(stream, "%s", text_after );

    log_end_header   ();
}

static void Tree_dump_tex_dfs_with_value(Tree_node *node, bool bracket, FILE *const stream, const double x_val)
{
    assert(node   != nullptr);
    assert(stream != nullptr);

    if (node->type == NODE_VAR)
    {
        if (bracket) fprintf(stream, "(");
        dump_tex_num(x_val,  stream);
        if (bracket) fprintf(stream, ")");
    }
    else Tree_dump_tex_dfs(node, bracket, stream);
}

/*_____________________________________________________________________*/

void Tex_tree_with_value(Tree_node *root, FILE *const stream,   const char *text_before,
                                                                const char *text_after , const double x_val)
{
    Tex_tree(root, stream, text_before, text_after, true, x_val);
}

void Tex_tree(Tree_node *root, FILE *const stream,  const char *text_before,
                                                    const char *text_after)
{
    Tex_tree(root, stream, text_before, text_after, false, 0);
}

/*_____________________________________________________________________*/

void Tex_message(FILE *const stream, const char *fmt, ...)
{
    assert(stream != nullptr);
    assert(fmt    != nullptr);

    va_list  ap;
    va_start(ap, fmt);

    vfprintf(stream, fmt, ap);
    
    va_end(ap);
}

void Tex_end(FILE *const stream)
{
    assert(stream != nullptr);

    fprintf(stream, "\\end{document}\n");
    fclose (stream);
}

/*_____________________________________________________________________*/

double get_value_in_point(Tree_node *root, const double x_val)
{
    log_header(__PRETTY_FUNCTION__);

    if (Tree_verify(root) == false)
    {
        log_error("Can't get value in the point of invalid tree.\n");

        Tree_dump_graphviz(root);
        return 0;
    }

    log_end_header();
    return dfs_value_in_point(root, x_val);
}

static double dfs_value_in_point(const Tree_node *node, const double x_val)
{
    assert(node != nullptr);

    if (node->type == NODE_VAR) return x_val;
    if (node->type == NODE_NUM) return getDBL;

    double left  = dfs_value_in_point(getL, x_val);
    double right = dfs_value_in_point(getR, x_val);

    return Tree_counter(left, right, getOP);
}

/*_____________________________________________________________________*/

//___________________

#undef getL
#undef getR
#undef getP
#undef getOP
#undef getDBL
#undef getVAR

//___________________
