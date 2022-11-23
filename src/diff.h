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
    OP_SIN      ,
    OP_COS      ,
    OP_TAN      ,
    OP_POW      ,
    OP_LOG      ,
    OP_SQRT     ,
    OP_SH       ,
    OP_CH       ,
    OP_ASIN     ,
    OP_ACOS     ,
    OP_ATAN     ,
};

enum VAR
{
    X       ,
    Y       ,
    Z       ,
    DX      ,
    DY      ,
    DZ      ,
    ALPHA   ,
    BETA    ,
    GAMMA   ,
    LAMBDA  ,
    MU      ,
};

struct Tree_node
{
    TYPE_NODE type;

    Tree_node * left;
    Tree_node *right;
    Tree_node * prev;

    unsigned int tree_vars;

    union
    {
        double      dbl;
        TYPE_OP      op;
        VAR         var;
    }
    value;
};

/*______________________________________FUNCTIONS_______________________________________*/

void        node_op_ctor            (Tree_node *const node, TYPE_OP             value          ,
                                                            Tree_node *const     left = nullptr,
                                                            Tree_node *const    right = nullptr,
                                                            Tree_node *const     prev = nullptr);

void        node_num_ctor           (Tree_node *const node, const double        value          ,
                                                            Tree_node *const     prev = nullptr);
void        node_var_ctor           (Tree_node *const node, VAR                 value          ,
                                                            Tree_node *const     prev = nullptr);

void        node_undef_ctor         (Tree_node *const node, Tree_node *const     prev = nullptr);

Tree_node  *new_node_op             (TYPE_OP   value,       Tree_node *const     prev = nullptr);
Tree_node  *new_node_op             (TYPE_OP   value           ,
                                     Tree_node *left           ,
                                     Tree_node *right          ,
                                     Tree_node *prev  = nullptr);

Tree_node  *new_node_num            (const double value, Tree_node *const prev = nullptr);
Tree_node  *new_node_var            (VAR          value, Tree_node *const prev = nullptr);
Tree_node  *new_node_undef          (                    Tree_node *const prev = nullptr);

void        node_dtor               (Tree_node *const node);
void        Tree_dtor               (Tree_node *const root);

bool        Tree_parsing_main       (Tree_node *const root, const char *file);
void        Tree_optimize_main      (Tree_node **     root);
Tree_node  *diff_main               (Tree_node **     root, const char *vars = "a");
void        Tree_optimize_var_main  (Tree_node *root, Tree_node *system_vars[]);

double      get_value_in_point      (Tree_node *root, const double x_val);

void        Tree_dump_graphviz      (Tree_node *root);
void        Tree_dump_txt           (Tree_node *root);
void        Tree_dump_tex           (Tree_node *root);

void        Tex_head                (const char *file, FILE **stream);
void        Tex_tree                (Tree_node  *root, FILE *const stream,  const char *text_before = nullptr,
                                                                            const char *text_after  = nullptr);
void        Tex_tree_with_value     (Tree_node   *root, FILE *const stream, const char *text_before,
                                                                            const char *text_after , const double x_val);

void        Tex_message             (FILE *const stream, const char *fmt, ...);
void        Tex_end                 (FILE *const stream);

/*______________________________________________________________________________________*/

#endif //DIFF_H