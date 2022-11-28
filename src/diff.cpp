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
//--------------------------------------------------------------------------------------------------------------------------
static void         dfs_dtor                (Tree_node *const node);
//--------------------------------------------------------------------------------------------------------------------------
static bool         Tree_parsing_execute    (Tree_node *const root, const char *data     ,
                                                                    const int   data_size,
                                                                    int *const  data_pos );
static Tree_node   *parse_general           (const char **data);
static Tree_node   *parse_op_add_sub        (const char **data);
static Tree_node   *parse_op_mul_div        (const char **data);
static Tree_node   *parse_op_pow            (const char **data);
static Tree_node   *parse_expretion         (const char **data);
static Tree_node   *parse_dbl               (const char **data);
//--------------------------------------------------------------------------------------------------------------------------
static void         Tree_optimize_execute   (Tree_node **node);
static bool         Tree_optimize_numbers   (Tree_node *node);
static bool         Tree_optimize_add_main  (Tree_node **node);
static bool         Tree_optimize_sub_main  (Tree_node **node);
static bool         Tree_optimize_mul_main  (Tree_node **node);
static bool         Tree_optimize_div_main  (Tree_node **node);
static bool         Tree_optimize_pow_main  (Tree_node **node);
static void         Tree_optimize_all       (Tree_node **node, Tree_node **null_son, Tree_node **good_son);
//--------------------------------------------------------------------------------------------------------------------------
static double       Tree_counter            (const double left, const double right, TYPE_OP op);
static bool         is_char_var             (const char c);
static VAR          get_diff_var            (VAR var);
//--------------------------------------------------------------------------------------------------------------------------
static Tree_node   *diff_execute            (Tree_node *const node, Tree_node *system_vars[], VAR var, bool d_mode);
static Tree_node   *diff_var_case           (Tree_node *const node,                           VAR var, bool d_mode);
static Tree_node   *diff_sys_case           (Tree_node *const node, Tree_node *system_vars[], VAR var, bool d_mode);
static Tree_node   *diff_op_case            (Tree_node *const node, Tree_node *system_vars[], VAR var, bool d_mode);
static Tree_node   *diff_op_pow             (Tree_node *const node, Tree_node *system_vars[], VAR var, bool d_mode);
static Tree_node   *Tree_copy               (Tree_node *cp_from);
//--------------------------------------------------------------------------------------------------------------------------
static void Tree_optimize_var_execute(Tree_node *node, Tree_node *system_vars[], int *const vars_index   ,
                                                                                 int *const tree_num_node,
                                                                                 int *const tree_num_div ,
                                                                                 int *const tree_num_pow ,
                                                                                 int *const tree_num_sqrt, const int sys_size);
static void make_var_change          (Tree_node *node, Tree_node *system_vars[], int *const vars_index   , const int sys_size);
static bool get_system_var           (Tree_node *node, Tree_node *system_vars[], int *const vars_index, int *const needed_ind,
                                                                                                        const  int   sys_size);

static void Tree_optimize_var_default(int *const num_node , int *const num_div , int *const num_pow , int *const num_sqrt ,
                                      const int  num_node2, const int  num_div2, const int  num_pow2,  const int num_sqrt2);
static void Tree_optimize_var_div    (int *const num_node , int *const num_div , int *const num_pow , int *const num_sqrt ,
                                      const int  num_node2, const int  num_div2, const int  num_pow2,  const int num_sqrt2);
static void Tree_optimize_var_pow    (int *const num_node , int *const num_div , int *const num_pow , int *const num_sqrt ,
                                      const int  num_node2, const int  num_div2, const int  num_pow2,  const int num_sqrt2);
static void Tree_optimize_var_sqrt   (int *const num_node , int *const num_div , int *const num_pow , int *const num_sqrt ,
                                      const int  num_node2, const int  num_div2, const int  num_pow2,  const int num_sqrt2);

static bool Tree_cmp                 (Tree_node *first, Tree_node *second);
static bool Tree_node_cmp            (Tree_node *first, Tree_node *second);
static bool edge_cmp                 (Tree_node *first, Tree_node *second);
static bool value_cmp                (Tree_node *first, Tree_node *second);
//--------------------------------------------------------------------------------------------------------------------------
static double Tree_get_value_in_var  (Tree_node *node, Tree_node *system_vars[],    const double x_val,
                                                                                    const double y_val,
                                                                                    const double z_val);
static double Tree_get_value_in_sys  (Tree_node *node, Tree_node *system_vars[],    const double x_val,
                                                                                    const double y_val,
                                                                                    const double z_val);
//--------------------------------------------------------------------------------------------------------------------------
static void         Tree_dump_graphviz_dfs  (Tree_node *node, int *const node_number, FILE *const stream);
static void         Tree_node_describe      (Tree_node *node, int *const node_number, FILE *const stream);
static void         print_Tree_node         (Tree_node *node, int *const node_number, FILE *const stream,   GRAPHVIZ_COLOR fillcolor,
                                                                                                            GRAPHVIZ_COLOR     color, 
                                                                                                            const char    *node_type,
                                                                                                            const char        *value);
//--------------------------------------------------------------------------------------------------------------------------
static void         Tree_dump_txt_dfs           (Tree_node *node, bool bracket);
static void         dump_txt_sys                (Tree_node *node);
static void         dump_txt_num                (Tree_node *node);
static void         dump_txt_unary              (Tree_node *node);
//--------------------------------------------------------------------------------------------------------------------------
static bool Tree_get_bracket_dfs        (Tree_node *node, Tree_node *system_vars[], char *const buff    ,
                                                                                    int  *const buff_pos);
static bool Tree_get_bracket_op_bin     (Tree_node *node, Tree_node *system_vars[], char *const buff,
                                                                                    int  *const buff_pos);
static bool Tree_get_bracket_op_unary   (Tree_node *node, Tree_node *system_vars[], char *const buff,
                                                                                    int  *const buff_pos);
static bool Tree_get_bracket_case_var   (Tree_node *node, Tree_node *system_vars[], char *const buff,
                                                                                    int  *const buff_pos);
static bool Tree_get_bracket_case_sys   (Tree_node *node, Tree_node *system_vars[], char *const buff,
                                                                                    int  *const buff_pos);
//--------------------------------------------------------------------------------------------------------------------------
static void Tree_dump_tex_system  (char *const dump_tex, char *const dump_pdf, const int cur);

static void Tree_dump_tex_dfs     (Tree_node *node, bool bracket, FILE *const stream, bool           is_val = false  ,
                                                                                      Tree_node *sys_vars[] = nullptr,
                                   const double x_val = POISON,
                                   const double y_val = POISON,
                                   const double z_val = POISON);

static void Tree_dump_tex_var     (Tree_node *node, FILE *const stream, bool           is_val,
                                                                        Tree_node *sys_vars[],  const double x_val,
                                                                                                const double y_val,
                                                                                                const double z_val);
static void Tree_dump_tex_sys     (Tree_node *node, FILE *const stream, bool           is_val,
                                                                        Tree_node *sys_vars[],  const double x_val,
                                                                                                const double y_val,
                                                                                                const double z_val);
static void Tree_dump_tex_op_unary(Tree_node *node, FILE *const stream, bool           is_val,
                                                                        Tree_node *sys_vars[],  const double x_val,
                                                                                                const double y_val,
                                                                                                const double z_val);

static void Tree_dump_tex_op_sqrt (Tree_node *node, FILE *const stream, bool           is_val,
                                                                        Tree_node *sys_vars[],  const double x_val,
                                                                                                const double y_val,
                                                                                                const double z_val);
static void Tree_dump_tex_op_div  (Tree_node *node, FILE *const stream, bool           is_val,
                                                                        Tree_node *sys_vars[],  const double x_val,
                                                                                                const double y_val,
                                                                                                const double z_val);

static void Tree_dump_tex_op_pow  (Tree_node *node, FILE *const stream, bool           is_val,
                                                                        Tree_node *sys_vars[],  const double x_val,
                                                                                                const double y_val,
                                                                                                const double z_val);

static bool Tree_dump_tex_op_sub  (Tree_node *node, FILE *const stream, bool           is_val,
                                                                        Tree_node *sys_vars[],  const double x_val,
                                                                                                const double y_val,
                                                                                                const double z_val);
//--------------------------------------------------------------------------------------------------------------------------
static void dump_tex_num          (Tree_node *node, FILE *const stream);
static void dump_tex_sys          (Tree_node *node, FILE *const stream);

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
    "\\cdot", // OP_MUL
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

const char *plot_names[] =
{
    "+"     ,
    "-"     ,
    "*"     ,
    "/"     ,
    "sin"   ,
    "cos"   ,
    "tan"   ,
    "**"    ,
    "log"   ,
    "sqrt"  ,
    "sinh"  ,
    "cosh"  ,
    "asin"  ,
    "acos"  ,
    "atan"  ,
};

const char *var_names[] =
{
    "x"             ,
    "y"             ,
    "z"             ,
    "dx"            ,
    "dy"            ,
    "dz"            ,
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
"\\documentclass[12pt,a4paper,fleqn]{article}\n"
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
"\\textwidth=180mm\n"
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
#define getSYS          sys(node)

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
    else if (node->type == NODE_SYS && !is_terminal_node) (*err) = (*err) | (1 << NON_TERMINAL_VAR);
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

    p(getL) = node;
    p(getR) = node;

    if (getL != nullptr &&
        getR != nullptr   )
    {
        p(getL) = node;
        p(getR) = node;
    }
}

void node_num_ctor(Tree_node *const node,   const double    value,
                                            Tree_node *const prev)
{
    assert(node != nullptr);

    node->type = NODE_NUM;
    
    getL      =  nullptr;
    getR      =  nullptr;
    getP      =     prev;
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
    getVAR    =      value;
}

void node_sys_ctor(Tree_node *const node,   int             value,
                                            Tree_node *const prev)
{
    assert(node != nullptr);

    node->type = NODE_SYS;

    getL    = nullptr;
    getR    = nullptr;
    getP    =    prev;
    getSYS  =   value;
}

void node_undef_ctor(Tree_node *const node, Tree_node *const prev)
{
    assert (node != nullptr);

    *node  = default_node;
    getP   =         prev;
}

//___________________

#undef getL
#undef getR
#undef getP
#undef getOP
#undef getDBL
#undef getVAR
#undef getSYS

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

Tree_node *new_node_sys(int value, Tree_node *const prev)
{
    Tree_node *new_node = (Tree_node *) log_calloc(1, sizeof(Tree_node));
    if        (new_node == nullptr) return nullptr;

    node_sys_ctor(new_node, value, prev);
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
#define getSYS sys(node)

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
#undef getSYS

//___________________

/*_____________________________________________________________________*/

static bool is_char_var(const char c)
{
    return c == 'x' || c == 'y' || c == 'z';
}

static VAR get_diff_var(VAR var)
{
    switch (var)
    {
        case X  : return DX;
        case Y  : return DY;
        case Z  : return DZ;
        default : assert(false && "default case in get_diff_var()");
    }
    return DX;
}

/*_____________________________________________________________________*/

Tree_node *Tree_parsing_buff(const char *buff)
{
    log_header(__PRETTY_FUNCTION__);

    if (buff == nullptr)
    {
        log_error("The buff is nullptr.\n");
        log_end_header();
        return nullptr;
    }

    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    log_message("buff = %p.\n", buff);
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    Tree_node *ret = parse_general(&buff);
    
    if (ret == nullptr)
    {
        log_error("Syntax_error in download file.\n");
        log_end_header();
        return nullptr;
    }
    log_message   (GREEN "Parsing successful.\n" CANCEL);
    log_end_header();
    return ret;
}

Tree_node *Tree_parsing_main(const char *file)
{
    log_header(__PRETTY_FUNCTION__);

    assert(file != nullptr);

    int         data_size = 0;
    const char *data      = (char *) read_file(file, &data_size);

    if (data == nullptr)
    {
        log_error     ("Can't open the file\n");
        log_end_header();
        return nullptr;
    }

    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    log_message("data = %p.\n", data);
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    const char *data_copy = data; //the original value of "data" pointer is needed to free the memory
    Tree_node *ret = parse_general(&data_copy);

    if (ret == nullptr)
    {
        log_error("Syntax_error in download file.\n");
        
        log_free      ((char *) data);
        log_end_header();
        return nullptr;
    }
    log_free      ((char *) data);
    log_message   (GREEN "Parsing successful.\n" CANCEL);
    log_end_header();
    return ret;
}

#define descent_err_check(node) if (node == nullptr) { return nullptr; }

static Tree_node *parse_general(const char **data)
{
    assert( data != nullptr);
    assert(*data != nullptr);

    Tree_node *ret = parse_op_add_sub(data);
    descent_err_check(ret);

    if (**data == '\n') return ret;
    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    log_error("general: last char != \'\\n\'.\n");
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    return nullptr;
}

static Tree_node *parse_op_add_sub(const char **data)
{
    assert( data != nullptr);
    assert(*data != nullptr);

    Tree_node *val = parse_op_mul_div(data);
    descent_err_check(val);

    while (**data == '+' || **data == '-')
    {
        TYPE_OP cur_op = (**data == '+') ? OP_ADD : OP_SUB;
        *data += 1;

        Tree_node *val2 = parse_op_mul_div(data);
        if (val2 == nullptr) { Tree_dtor(val); return nullptr; }

        val = new_node_op(cur_op, val, val2);
    }
    return val;
}

static Tree_node *parse_op_mul_div(const char **data)
{
    assert( data != nullptr);
    assert(*data != nullptr);

    Tree_node *val = parse_op_pow(data);
    descent_err_check(val);

    while (**data == '*' || **data == '/')
    {
        TYPE_OP cur_op = (**data == '*') ? OP_MUL : OP_DIV;
        *data += 1;

        Tree_node *val2 = parse_op_pow(data);
        if (val2 == nullptr) { Tree_dtor(val); return nullptr; }

        val = new_node_op(cur_op, val, val2);
    }
    return val;
}

static Tree_node *parse_op_pow(const char **data)
{
    assert( data != nullptr);
    assert(*data != nullptr);

    Tree_node *val = parse_expretion(data);
    descent_err_check(val);

    if (**data == '^')
    {
        TYPE_OP cur_op = OP_POW;
        *data += 1;

        Tree_node *val2 = parse_expretion(data);
        if (val2 == nullptr) { Tree_dtor(val); return nullptr; }

        val = new_node_op(cur_op, val, val2);
    }
    return val;
}

//___________________

#define UNARY(str, n, op_val)                                   \
    if (!strncmp(*data, str, n))                                \
    {                                                           \
        *data += n;                                             \
                                                                \
        val = parse_op_add_sub(data);                           \
        descent_err_check(val);                                 \
                                                                \
        if (**data != ')')                                      \
        {/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/                      \
            log_error("no closed bracket on %p.\n", *data);     \
         /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/                      \
            Tree_dtor(val);                                     \
            return nullptr;                                     \
        }                                                       \
        *data += 1;                                             \
                                                                \
        return new_node_op(op_val, Nul, val);                   \
    }

//___________________

static Tree_node *parse_expretion(const char **data)
{
    assert( data != nullptr);
    assert(*data != nullptr);

    Tree_node *val = nullptr;

    if (**data == '(')
    {
       *data += 1;
        val = parse_op_add_sub(data);
        
        if (**data != ')')
        {
            //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
            log_error("no closed bracket on %p.\n", *data);
            //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
            Tree_dtor(val);
            return nullptr;
        }
        *data += 1;

        return val;
    }

    #include "diff_gen.h"

    if (**data == 'x') { *data += 1; return new_node_var(X); }
    if (**data == 'y') { *data += 1; return new_node_var(Y); }
    if (**data == 'z') { *data += 1; return new_node_var(Z); }
    if (**data == 'e') { *data += 1; return new_node_num(e); }

    return parse_dbl(data);
}

//___________________

#undef UNARY

//___________________

static Tree_node *parse_dbl(const char **data)
{
    assert( data != nullptr);
    assert(*data != nullptr);

    double val = 0;
    const char *s_before = *data;

    val = strtod(*data, (char **) data);
    if (val == HUGE_VAL || s_before == *data)
    {   //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
        log_error("wrong double on %p.\n", *data);
        //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
        return nullptr;
    }

    return new_node_num(val);
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

    Tree_optimize_execute( root);
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
#define getSYS sys(node)

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
#undef getSYS

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
#define getSYS sys(*node)

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

//___________________

#undef getL
#undef getR
#undef getP
#undef getOP
#undef getDBL
#undef getVAR
#undef getSYS

//___________________

/*_____________________________________________________________________*/


Tree_node *diff_main(Tree_node **root, Tree_node *system_vars[], const char *vars)
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
        case 'x': diff_root = diff_execute(*root, system_vars, X, false);
                  break;
        case 'y': diff_root = diff_execute(*root, system_vars, Y, false);
                  break;
        case 'z': diff_root = diff_execute(*root, system_vars, Z, false);
                  break;
        default : diff_root = Add(diff_execute(*root, system_vars, X, true),
                              Add(diff_execute(*root, system_vars, Y, true),
                                  diff_execute(*root, system_vars, Z, true)));
                  break;
    }
    return diff_root;
}

//_____________________________

#define DL dL(node, system_vars, var, d_mode)
#define DR dR(node, system_vars, var, d_mode)
#define CL cL(node)
#define CR cR(node)

//_____________________________

static Tree_node *diff_execute(Tree_node *const node, Tree_node *system_vars[], VAR var, bool d_mode)
{
    assert(node != nullptr);

    switch(node->type)
    {
        case NODE_NUM  : return Nul;
        
        case NODE_VAR  : return diff_var_case(node,              var, d_mode);        
        case NODE_OP   : return diff_op_case (node, system_vars, var, d_mode);
        case NODE_SYS  : return diff_sys_case(node, system_vars, var, d_mode);
        
        case NODE_UNDEF:
        default        : log_error      ("default case in diff_execute() in TYPE-NODE-switch: node_type = %d.\n", node->type);
                         Tree_dump_graphviz(node);
                         assert(false && "default case in TYPE_NODE-switch");
                         return nullptr;
    }

    return nullptr;
}

static Tree_node *diff_sys_case(Tree_node *const node, Tree_node *system_vars[], VAR var, bool d_mode)
{
    assert(node        !=  nullptr);
    assert(node->type  == NODE_SYS);

    assert(system_vars            != nullptr);
    assert(system_vars[sys(node)] != nullptr);

    return diff_execute(system_vars[sys(node)], system_vars, var, d_mode);
}

static Tree_node *diff_var_case(Tree_node *const node, VAR var, bool d_mode)
{
    assert(node       != nullptr);
    assert(node->type == NODE_VAR);

    if (var(node) == var)
    {
        if (d_mode) return new_node_var(get_diff_var(var));
        return Num(1);
    }
    return Num(0);
}

static Tree_node *diff_op_case(Tree_node *const node, Tree_node *system_vars[], VAR var, bool d_mode)
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

        case OP_POW : return diff_op_pow(node, system_vars, var, d_mode);

        case OP_SQRT: return Div(DR, Mul(Num(2), Sqrt(CL, CR)));

        case OP_SH  : return Mul(Ch(CL, CR), DR);
        case OP_CH  : return Mul(Sh(CL, CR), DR);

        case OP_ASIN: return         Div(DR, Sqrt(Nul, Sub(Num(1), Pow(CR, Num(2)))));
        case OP_ACOS: return Sub(0 , Div(DR, Sqrt(Nul, Sub(Num(1), Pow(CR, Num(2))))));
        case OP_ATAN: return Div(DR,                   Add(Num(1), Pow(CR, Num(2))));

        default     : log_error      ("default case in diff_execute() in TYPE-OP-switch: op_type = %d.\n", op(node));
                      Tree_dump_graphviz(node);
                      assert(false && "default case in diff_execute() in TYPE_OP-switch");
                      return nullptr;
    }
    return nullptr;
}

static Tree_node *diff_op_pow(Tree_node *const node, Tree_node *system_vars[], VAR var, bool d_mode)
{
    assert(node       != nullptr);
    assert(node->type == NODE_OP);
    assert(op(node)   ==  OP_POW);

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

static Tree_node *Tree_copy(Tree_node *cp_from)
{
    assert(cp_from != nullptr);

    switch (cp_from->type)
    {
        case NODE_NUM   : return Num(dbl(cp_from));

        case NODE_VAR   : return new_node_var(var(cp_from));
        case NODE_SYS   : return new_node_sys(sys(cp_from));
        case NODE_OP    : return new_node_op (op (cp_from), cL(cp_from), cR(cp_from));

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
#define getSYS sys(node)

const int MAX_NODE = 25;
const int MAX_DIV  =  1;
const int MAX_POW  =  1;
const int MAX_SQRT =  2;

//___________________

void Tree_optimize_var_main(Tree_node **root, Tree_node *system_vars[], const int sys_size)
{
    log_header(__PRETTY_FUNCTION__);


    if (Tree_verify(*root) == false)
    {
        log_error("Can't do optimize_var, because the tree is invalid.\n");
        return;
    }
    if (system_vars == nullptr)
    {
        log_error("system_vars is nullptr.\n");
        return;
    }

    int    vars_index = 0;
    while (vars_index < sys_size && system_vars[vars_index] != nullptr) ++vars_index;

    int num_node   =     0;
    int num_div    =     0;
    int num_pow    =     0;
    int num_sqrt   =     0;

    Tree_optimize_main(root);
    Tree_optimize_var_execute(*root, system_vars, &vars_index, &num_node, &num_div, &num_pow, &num_sqrt, sys_size);

    log_end_header();
}

static void Tree_optimize_var_execute(Tree_node *node, Tree_node *system_vars[], int *const vars_index   ,
                                                                                 int *const tree_num_node,
                                                                                 int *const tree_num_div ,
                                                                                 int *const tree_num_pow ,
                                                                                 int *const tree_num_sqrt, const int sys_size)
{
    assert(node        != nullptr);
    assert(system_vars != nullptr);
    assert(vars_index  != nullptr);

    assert(tree_num_node != nullptr);
    assert(tree_num_div  != nullptr);
    assert(tree_num_pow  != nullptr);
    assert(tree_num_sqrt != nullptr);

    if (node->type == NODE_OP)
    {
        int subtree_num_node = 0;
        int subtree_num_div  = 0;
        int subtree_num_pow  = 0;
        int subtree_num_sqrt = 0;
        Tree_optimize_var_execute(getL, system_vars, vars_index, tree_num_node,
                                                                 tree_num_div ,
                                                                 tree_num_pow ,
                                                                 tree_num_sqrt, sys_size);

        Tree_optimize_var_execute(getR, system_vars, vars_index, &subtree_num_node,
                                                                 &subtree_num_div ,
                                                                 &subtree_num_pow ,
                                                                 &subtree_num_sqrt, sys_size);
        switch (getOP)
        {
            case OP_DIV : Tree_optimize_var_div        (   tree_num_node,    tree_num_div,    tree_num_pow,    tree_num_sqrt,
                                                        subtree_num_node, subtree_num_div, subtree_num_pow, subtree_num_sqrt);
                          break;
            case OP_POW : Tree_optimize_var_pow        (   tree_num_node,    tree_num_div,    tree_num_pow,    tree_num_sqrt,
                                                        subtree_num_node, subtree_num_div, subtree_num_pow, subtree_num_sqrt);
                          break;
            case OP_SQRT: Tree_optimize_var_sqrt       (   tree_num_node,    tree_num_div,    tree_num_pow,    tree_num_sqrt,
                                                        subtree_num_node, subtree_num_div, subtree_num_pow, subtree_num_sqrt);
                          break;
            default     : Tree_optimize_var_default    (   tree_num_node,    tree_num_div,    tree_num_pow,    tree_num_sqrt,
                                                        subtree_num_node, subtree_num_div, subtree_num_pow, subtree_num_sqrt);
                          break;
        }

        if (*tree_num_node > MAX_NODE ||
            *tree_num_div  > MAX_DIV  ||
            *tree_num_pow  > MAX_POW  ||
            *tree_num_sqrt > MAX_SQRT   )
        {
            make_var_change(node, system_vars, vars_index, sys_size);
            *tree_num_node = 1; //if there no the replacement of node, this parametres become useless
            *tree_num_div  = 0;
            *tree_num_pow  = 0;
            *tree_num_sqrt = 0;
        }
    }
    else *tree_num_node = 1; //node->type != NODE_OP
}

static void make_var_change(Tree_node *node, Tree_node *system_vars[], int *const vars_index, const int sys_size)
{
    assert(node        != nullptr);
    assert(system_vars != nullptr);
    assert(vars_index  != nullptr);

    if (*vars_index == sys_size) return;
    if (getP        ==  nullptr) return;

    int system_var_ind = 0;
    bool is_new_var    = get_system_var(node, system_vars, vars_index, &system_var_ind, sys_size);

    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    log_message(GREEN   "system_var_ind = %d\n"
                        "is_new_var     = %d\n"
                        "node           = %p\n\n" CANCEL, system_var_ind, is_new_var, node);
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    if (r(getP) == node)
    {
        r(getP) = new_node_sys(system_var_ind, getP);
        
        if (!is_new_var) Tree_dtor(node); // delete the node, because there was the duplicate of it before
    }
    else
    {
        l(getP) = new_node_sys(system_var_ind, getP);

        if (!is_new_var) Tree_dtor(node); // delete the node, because there was the duplicate of it before
    }
}

static bool get_system_var(Tree_node *node, Tree_node *system_vars[], int *const vars_index, int *const needed_ind,
                                                                                             const int  sys_size  )
{
    assert(needed_ind  != nullptr);
    assert( vars_index != nullptr);
    assert(*vars_index < sys_size);

    for (int cnt = 0; cnt < *vars_index; ++cnt)
    {
        if (Tree_cmp(node, system_vars[cnt]))
        {
            *needed_ind = cnt;
            return false;
        }
    }

    system_vars  [*vars_index] = node;
    *needed_ind = *vars_index;
    *vars_index += 1;

    return true;
}

static void Tree_optimize_var_default(int *const num_node , int *const num_div , int *const num_pow , int *const num_sqrt ,
                                      const int  num_node2, const int  num_div2, const int  num_pow2,  const int num_sqrt2)
{
    assert(num_node != nullptr);
    assert(num_div  != nullptr);
    assert(num_pow  != nullptr);
    assert(num_sqrt != nullptr);

    *num_node += num_node2 + 1;
    *num_div   = get_max(*num_div , num_div2 );
    *num_pow   = 0;
    *num_sqrt  = get_max(*num_sqrt, num_sqrt2);
}

static void Tree_optimize_var_div(int *const num_node , int *const num_div , int *const num_pow , int *const num_sqrt ,
                                  const int  num_node2, const int  num_div2, const int  num_pow2,  const int num_sqrt2)
{
    assert(num_node != nullptr);
    assert(num_div  != nullptr);
    assert(num_pow  != nullptr);
    assert(num_sqrt != nullptr);

    *num_node = get_max(*num_node, num_node2);
    *num_div += num_div2 + 1;
    *num_pow  = 0;
    *num_sqrt = get_max(*num_sqrt, num_sqrt2);
}

static void Tree_optimize_var_pow(int *const num_node , int *const num_div , int *const num_pow , int *const num_sqrt ,
                                  const int  num_node2, const int  num_div2, const int  num_pow2,  const int num_sqrt2)
{
    assert(num_node != nullptr);
    assert(num_div  != nullptr);
    assert(num_pow  != nullptr);
    assert(num_sqrt != nullptr);

    *num_node += num_node2 + 1;
    *num_div   = get_max(*num_div, num_div2);
    *num_pow  += num_pow2 + 1;
    *num_sqrt += num_sqrt2;
}

static void Tree_optimize_var_sqrt(int *const num_node , int *const num_div , int *const num_pow , int *const num_sqrt ,
                                   const int  num_node2, const int  num_div2, const int  num_pow2,  const int num_sqrt2)
{
    assert(num_node != nullptr);
    assert(num_div  != nullptr);
    assert(num_pow  != nullptr);
    assert(num_sqrt != nullptr);

    *num_node += num_node2 + 1;
    *num_div   = get_max(*num_div, num_div2);
    *num_pow   = get_max(*num_pow, num_pow2);
    *num_sqrt += num_sqrt2 + 1;
}

static bool Tree_cmp(Tree_node *first, Tree_node *second)
{
    assert(first  != nullptr);
    assert(second != nullptr);
    
    if (Tree_node_cmp(first, second))
    {
        if (first->type == NODE_OP)
        {
            if (op(first) == OP_ADD || op(first) == OP_MUL)
            {
                if (l(first) != nullptr)
                {
                    return  (Tree_cmp(l(first), l(second)) && Tree_cmp(r(first), r(second))) ||
                            (Tree_cmp(l(first), r(second)) && Tree_cmp(r(first), l(second)));
                }
                return true;
            }
            return l(first) == nullptr || (Tree_cmp(l(first), l(second)) && Tree_cmp(r(first), r(second)));
        }
        return true;
    }
    return false;
}

static bool Tree_node_cmp(Tree_node *first, Tree_node *second)
{
    assert(first  != nullptr);
    assert(second != nullptr);

    if (value_cmp(first, second)              &&
        edge_cmp(l(first), l(second))         &&
        edge_cmp(r(first), r(second))
        ) 
        return true;

    return false;
}

static bool edge_cmp(Tree_node *first, Tree_node *second)
{
    if (first == nullptr && second == nullptr) return true;
    if (first != nullptr && second != nullptr) return true;
    return false;
}

static bool value_cmp(Tree_node *first, Tree_node *second)
{
    assert(first  != nullptr);
    assert(second != nullptr);

    if (first->type != second->type) return false;

    switch (first->type)
    {
        case NODE_OP : return  op(first) ==  op(second);
        case NODE_NUM: return approx_equal(dbl(first), dbl(second));
        case NODE_VAR: return var(first) == var(second);
        case NODE_SYS: return sys(first) == sys(second);

        default      : assert(false && "default case in value_cmp()\n");
    }
    return false;
}

/*_____________________________________________________________________*/

double Tree_get_value_in_point(Tree_node *node, Tree_node *system_vars[],   const double x_val,
                                                                            const double y_val,
                                                                            const double z_val)
{
    assert(node != nullptr);

    switch (node->type)
    {
        case NODE_NUM: return getDBL;
        case NODE_VAR: return Tree_get_value_in_var(node, system_vars, x_val, y_val, z_val);
        case NODE_SYS: return Tree_get_value_in_sys(node, system_vars, x_val, y_val, z_val);

        case NODE_OP :  {
                            double left  = Tree_get_value_in_point(getL, system_vars, x_val, y_val, z_val);
                            double right = Tree_get_value_in_point(getR, system_vars, x_val, y_val, z_val);
                            
                            return Tree_counter(left, right, getOP);
                        }
    }
    return 0;
}

static double Tree_get_value_in_var(Tree_node *node, Tree_node *system_vars[],  const double x_val,
                                                                                const double y_val,
                                                                                const double z_val)
{
    assert(node       !=  nullptr);
    assert(node->type == NODE_VAR);

    switch (getVAR)
    {
        case X : return x_val;
        case Y : return y_val;
        case Z : return z_val;

        case DX:
        case DY:
        case DZ: log_error("Can't get value in diff_node.\n");
                 return 0;

        default: log_error      ("default case in Tree_get_value_in_var(): gatVar = %d.\n", getVAR);
                 assert(false && "default case in Tree_get_value_in_var()");
                 break;
    }
    return 0;
}

static double Tree_get_value_in_sys(Tree_node *node, Tree_node *system_vars[],  const double x_val,
                                                                                const double y_val,
                                                                                const double z_val)
{
    assert(node != nullptr);
    assert(node->type == NODE_SYS);

    assert(system_vars         != nullptr);
    assert(system_vars[getSYS] != nullptr);

    return Tree_get_value_in_point(system_vars[getSYS], system_vars, x_val, y_val, z_val);
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
        case NODE_VAR:
                            color =  DARK_BLUE;
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

        case NODE_SYS:      color = GOLD;
                        fillcolor = YELLOW_HTML;
                        node_type =  "NODE_SYS";
                        sprintf(value, "sys: %d", getSYS);
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

    if      (node->type == NODE_NUM) dump_txt_num(node);
    else if (node->type == NODE_SYS) dump_txt_sys(node);
    else if (node->type == NODE_VAR) log_message ("%s", var_names[getVAR]);
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

static void dump_txt_sys(Tree_node *node)
{
    assert(node       != nullptr);
    assert(node->type == NODE_SYS);

    log_message("x_%d", sys(node));
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

//___________________

#define Tree_dump_tex_dfs_make(node, bracket)                                                       \
        Tree_dump_tex_dfs     (node, bracket, stream, is_val, sys_vars, x_val, y_val, z_val)

#define Tree_dump_tex_var_make(node)                                                                \
        Tree_dump_tex_var     (node, stream, is_val, sys_vars, x_val, y_val, z_val)

#define Tree_dump_tex_op_unary_make(node)                                                           \
        Tree_dump_tex_op_unary     (node, stream, is_val, sys_vars, x_val, y_val, z_val)

#define Tree_dump_tex_sys_make(node)                                                                \
        Tree_dump_tex_sys     (node, stream, is_val, sys_vars, x_val, y_val, z_val)
#define Tree_dump_tex_op_sqrt_make(node)                                                            \
        Tree_dump_tex_op_sqrt     (node, stream, is_val, sys_vars, x_val, y_val, z_val)

#define Tree_dump_tex_op_div_make(node)                                                             \
        Tree_dump_tex_op_div     (node, stream, is_val, sys_vars, x_val, y_val, z_val)

#define Tree_dump_tex_op_pow_make(node)                                                             \
        Tree_dump_tex_op_pow     (node, stream, is_val, sys_vars, x_val, y_val, z_val)

#define Tree_dump_tex_op_sub_make(node)                                                             \
        Tree_dump_tex_op_sub     (node, stream, is_val, sys_vars, x_val, y_val, z_val)

//___________________

static void Tree_dump_tex_dfs(Tree_node *node, bool bracket, FILE *const stream, bool           is_val,
                                                                                 Tree_node *sys_vars[], const double x_val,
                                                                                                        const double y_val,
                                                                                                        const double z_val)
{
    assert(node   != nullptr);
    assert(stream != nullptr);

    if (bracket) fprintf(stream, "\\left(");

    if      (node->type == NODE_NUM) dump_tex_num          (node, stream);
    else if (node->type == NODE_VAR) Tree_dump_tex_var_make(node        );
    else if (node->type == NODE_SYS) Tree_dump_tex_sys_make(node        );
    else if (getOP      ==   OP_SUB && Tree_dump_tex_op_sub_make(node));
    else
    {
        switch (getOP)
        {
            case OP_DIV: Tree_dump_tex_op_div_make(node);
                         break;
            
            case OP_SIN :
            case OP_COS :
            case OP_TAN :
            case OP_SH  :
            case OP_CH  :
            case OP_ASIN:
            case OP_ACOS:
            case OP_ATAN:
            case OP_LOG : Tree_dump_tex_op_unary_make(node);
                          break;
            
            case OP_SQRT: Tree_dump_tex_op_sqrt_make(node);
                          break;

            case OP_POW : Tree_dump_tex_op_pow_make(node);
                          break;

            default     : {
                            Tree_dump_tex_dfs_make(getL, !(getL->type != NODE_OP ||  op_priority[op(getL)] >= 
                                                                                     op_priority[getOP]));
                            fprintf(stream, op_names[getOP]);

                            Tree_dump_tex_dfs_make(getR, !(getR->type != NODE_OP ||  op_priority[op(getR)] >
                                                                                     op_priority[getOP]));
                            break;
                          }
        }
    }
    if (bracket) fprintf(stream, "\\right)");
}

//___________________

#define tex_user_var(VAR_NAME, var_name)                        \
        if (getVAR == VAR_NAME)                                 \
        {                                                       \
            if (approx_equal(var_name##_val, POISON))           \
            {                                                   \
                fprintf(stream, " %s", var_names[VAR_NAME]);    \
            }                                                   \
            else dump_tex_num(var_name##_val, stream);          \
                                                                \
        return;                                                 \
        }

#define tex_diff_var(VAR_NAME)                                  \
        if (getVAR == VAR_NAME)                                 \
        {                                                       \
            fprintf(stream, " %s", var_names[VAR_NAME]);        \
            return;                                             \
        }

//___________________

static void Tree_dump_tex_var(Tree_node *node, FILE *const stream, bool           is_val,
                                                                   Tree_node *sys_vars[], const double x_val,
                                                                                          const double y_val,
                                                                                          const double z_val)
{
    assert(node       !=  nullptr);
    assert(stream     !=  nullptr);
    assert(node->type == NODE_VAR);

    if (is_val == false)
    {
        fprintf(stream, " %s", var_names[getVAR]);
        return;
    }

    tex_user_var(X, x)
    tex_user_var(Y, y)
    tex_user_var(Z, z)

    tex_diff_var(DX)
    tex_diff_var(DY)
    tex_diff_var(DZ)
}

//___________________

#undef tex_user_var
#undef tex_diff_var
#undef tex_system_var

//___________________

static void Tree_dump_tex_sys(Tree_node *node, FILE *const stream, bool           is_val,
                                                                    Tree_node *sys_vars[], const double x_val,
                                                                                           const double y_val,
                                                                                           const double z_val)
{
    assert(node       !=  nullptr);
    assert(stream     !=  nullptr);
    assert(node->type == NODE_SYS);

    if (is_val == false)
    {
        fprintf(stream, " x_{%d}", getSYS);
        return;
    }

    double dbl = Tree_get_value_in_point(node, sys_vars, x_val, y_val, z_val);
    dump_tex_num(dbl, stream);
    return;
}

static void Tree_dump_tex_op_unary(Tree_node *node, FILE *const stream, bool           is_val,
                                                                        Tree_node *sys_vars[],  const double x_val,
                                                                                                const double y_val,
                                                                                                const double z_val)
{
    assert(node       != nullptr);
    assert(stream     != nullptr);
    assert(node->type == NODE_OP);

    fprintf(stream, " %s", op_names[getOP]);
    
    if (getR->type == NODE_OP)  Tree_dump_tex_dfs_make(getR, true);
    else                        Tree_dump_tex_dfs_make(getR, false);
}

static void Tree_dump_tex_op_sqrt(Tree_node *node,  FILE *const stream, bool           is_val,
                                                                        Tree_node *sys_vars[],  const double x_val,
                                                                                                const double y_val,
                                                                                                const double z_val)
{
    assert(node       != nullptr);
    assert(stream     != nullptr);
    assert(node->type == NODE_OP);
    assert(getOP      == OP_SQRT);

    fprintf               (stream, "\\sqrt{");
    Tree_dump_tex_dfs_make(getR, false);
    fprintf               (stream, "}");
}

static void Tree_dump_tex_op_div(Tree_node *node,   FILE *const stream, bool           is_val,
                                                                        Tree_node *sys_vars[],  const double x_val,
                                                                                                const double y_val,
                                                                                                const double z_val)
{
    assert(node       != nullptr);
    assert(stream     != nullptr);
    assert(node->type == NODE_OP);
    assert(getOP      ==  OP_DIV);

    fprintf               (stream, "\\frac{");
    Tree_dump_tex_dfs_make(getL , false);
    fprintf               (stream, "}{");
    Tree_dump_tex_dfs_make(getR, false);
    fprintf               (stream, "}");
}

static void Tree_dump_tex_op_pow(Tree_node *node,   FILE *const stream, bool           is_val,
                                                                        Tree_node *sys_vars[],  const double x_val,
                                                                                                const double y_val,
                                                                                                const double z_val)
{
    assert(node       != nullptr);
    assert(stream     != nullptr);
    assert(node->type == NODE_OP);
    assert(getOP      ==  OP_POW);

    if (getL->type == NODE_OP) Tree_dump_tex_dfs_make(getL, true);
    else                       Tree_dump_tex_dfs_make(getL, false);

    fprintf               (stream, "^{");
    Tree_dump_tex_dfs_make(getR, false);
    fprintf               (stream, "}");
}

static bool Tree_dump_tex_op_sub(Tree_node *node,   FILE *const stream, bool           is_val,
                                                                        Tree_node *sys_vars[],  const double x_val,
                                                                                                const double y_val,
                                                                                                const double z_val)
{
    assert(node       != nullptr);
    assert(stream     != nullptr);
    assert(node->type == NODE_OP);
    assert(getOP      ==  OP_SUB);

    if (getL->type == NODE_NUM && approx_equal(0, dbl(getL)))
    {
        fprintf(stream, "-");
        Tree_dump_tex_dfs_make(getR, false);

        return true;
    }
    return false;
}

//___________________

#undef Tree_dump_tex_dfs_make
#undef Tree_dump_tex_op_unary
#undef Tree_dump_tex_op_sqrt
#undef Tree_dump_tex_op_div
#undef Tree_dump_tex_op_pow
#undef Tree_dump_tex_op_sub

//___________________

/*_____________________________________________________________________*/

static void dump_tex_num(Tree_node *node, FILE *const stream)
{
    assert(node       !=  nullptr);
    assert(node->type == NODE_NUM);

    dump_tex_num(dbl(node), stream);
}

void dump_tex_num(const double num, FILE *const stream)
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

void Tex_tree(Tree_node *root, FILE *const stream,  const char *text_before,
                                                    const char *text_after , Tree_node *system_vars[],
              double x_val,
              double y_val,
              double z_val)
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
    if (stream == nullptr)
    {
        log_error     ("Nullptr stream.\n");
        log_end_header();
        return;
    }

    if (text_before != nullptr) fprintf(stream, "%s", text_before);

    if (approx_equal(x_val, POISON) && approx_equal(y_val, POISON) && approx_equal(z_val, POISON))
    {
        Tree_dump_tex_dfs(root, false, stream, false, nullptr);
    }
    else
    {
        x_val = (approx_equal(x_val, POISON)) ? 0 : x_val;
        y_val = (approx_equal(y_val, POISON)) ? 0 : y_val;
        z_val = (approx_equal(z_val, POISON)) ? 0 : z_val;

        Tree_dump_tex_dfs(root, false, stream, true , system_vars, x_val, y_val, z_val);
    }

    if (text_after  != nullptr) fprintf(stream, "%s", text_after );

    log_end_header   ();
}

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

bool Tree_get_bracket_fmt(Tree_node *root, Tree_node *system_vars[], char *const buff)
{
    log_header(__PRETTY_FUNCTION__);

    if (Tree_verify(root) == false)
    {
        log_error     ("Invalid tree.\n");
        log_end_header();
        return false;
    }
    if (buff == nullptr)
    {
        log_error     ("buff is nullptr.\n");
        log_end_header();
        return false;
    }

    int buff_pos = 0;
    log_end_header();
    return Tree_get_bracket_dfs(root, system_vars, buff, &buff_pos);
}

static bool Tree_get_bracket_dfs(Tree_node *node, Tree_node *system_vars[], char *const buff    ,
                                                                            int  *const buff_pos)
{
    assert(node     != nullptr);
    assert(buff     != nullptr);
    assert(buff_pos != nullptr);

    switch(node->type)
    {
        case NODE_NUM:  {
                            int added = 0;
                            sprintf(buff + *buff_pos, "(%lg)%n", getDBL, &added);
                            *buff_pos += added;

                            return true;
                        }
        case NODE_VAR:  return Tree_get_bracket_case_var(node, system_vars, buff, buff_pos);
        case NODE_SYS:  return Tree_get_bracket_case_sys(node, system_vars, buff, buff_pos);

        case NODE_OP :  switch(getOP)
                        {
                            case OP_ADD:
                            case OP_SUB:
                            case OP_MUL:
                            case OP_DIV:
                            case OP_POW: return Tree_get_bracket_op_bin  (node, system_vars, buff, buff_pos);
                            default    : return Tree_get_bracket_op_unary(node, system_vars, buff, buff_pos);
                        }
        
        default      :  log_error(      "default case in Tree_get_brackets_dfs(): node->type = %d.\n", node->type);
                        assert(false && "default case in Tree_get_brackets_dfs()");
                        return false;
    }
    return false;
}

static bool Tree_get_bracket_op_bin(Tree_node *node, Tree_node *system_vars[],  char *const buff,
                                                                                int  *const buff_pos)
{
    assert(node     != nullptr);
    assert(buff     != nullptr);
    assert(buff_pos != nullptr);

    sprintf(buff + *buff_pos, "(");
    *buff_pos += 1;

    if (Tree_get_bracket_dfs(getL, system_vars, buff, buff_pos))
    {
        int added = 0;
        sprintf(buff + *buff_pos, "%s%n", plot_names[getOP], &added);
        *buff_pos += added;
    }
    else return false;

    if (Tree_get_bracket_dfs(getR, system_vars, buff, buff_pos))
    {
        int added = 0;
        sprintf(buff + *buff_pos, ")");
        *buff_pos += 1;
    }
    else return false;

    return true;
}

static bool Tree_get_bracket_op_unary(Tree_node *node, Tree_node *system_vars[],  char *const buff,
                                                                                  int  *const buff_pos)
{
    assert(node     != nullptr);
    assert(buff     != nullptr);
    assert(buff_pos != nullptr);

    int added = 0;
    sprintf(buff + *buff_pos, "(%s%n", plot_names[getOP], &added);
    *buff_pos += added;

    if (Tree_get_bracket_dfs(getR, system_vars, buff, buff_pos))
    {
        sprintf(buff + *buff_pos, ")");
        *buff_pos += 1;
    }
    return true;
}

static bool Tree_get_bracket_case_var(Tree_node *node, Tree_node *system_vars[], char *const buff,
                                                                                 int  *const buff_pos)
{
    assert(node       !=  nullptr);
    assert(buff       !=  nullptr);
    assert(buff_pos   !=  nullptr);
    assert(node->type == NODE_VAR);

    int added = 0;
    sprintf(buff + *buff_pos, "(%s)%n", var_names[getVAR], &added);
    *buff_pos += added;

    return true;
}

static bool Tree_get_bracket_case_sys(Tree_node *node, Tree_node *system_vars[], char *const buff,
                                                                                 int  *const buff_pos)
{
    assert(node       !=  nullptr);
    assert(buff       !=  nullptr);
    assert(buff_pos   !=  nullptr);
    assert(node->type == NODE_SYS);

    if (system_vars == nullptr)
    {
        log_error("system_vars is nullptr. Can't access the system variable.\n");
        return false;
    }
    if (system_vars[getVAR] == nullptr)
    {
        log_error("system_vars[VAR_NAME] is nullptr. Can't access the system variable.\n");
        return false;
    }

    return Tree_get_bracket_dfs(system_vars[getVAR], system_vars, buff, buff_pos);
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
