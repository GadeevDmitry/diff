#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>

#include "diff.h"
#include "../lib/logs/log.h"
#include "../lib/algorithm/algorithm.h"

static const int   TASK_SIZE =        100;
static const int   VARS_SIZE =        100;
static const int   BUFF_SIZE =       1000;
static       int    cur_task =          1;
static const char      *file = "math.tex";

char  task[TASK_SIZE] = {};

/*____________________________________*/

void       header            (FILE *const stream);
void       new_task          (FILE *const stream);
char      *get_task          ();
//---------------------------------------------------------------------------------------------------------------------------
void       Tex_system_vars   (Tree_node *system_vars[], FILE *const stream);
Tree_node *Teylor            (Tree_node **tree,         FILE *const stream, const char *text_f, const char *text_f_point,
                                                                                                double *const dbl);
//---------------------------------------------------------------------------------------------------------------------------
void Tree_plot      (Tree_node *root, Tree_node *root_vars[],
                     Tree_node *diff, Tree_node *diff_vars[],   const double x_min = POISON, const double x_max = POISON,
                                                                const double y_min = POISON, const double y_max = POISON);
bool func_init      (Tree_node *root, Tree_node *system_vars[], char *const bracket_fmt, bool is_diff);
void func_system    (const char *root_fmt, const char *diff_fmt);
void range_system   (const double min_val, const double max_val, const char c);

/*____________________________________*/


#define START()                                             \
        FILE      *stream    = nullptr;                     \
        Tree_node *tree      = nullptr;                     \
        Tree_node *tree_diff = nullptr;                     \
                                                            \
        Tex_head(file, &stream);                            \
        header         (stream);

#define _$$      fprintf(stream, "\n$$\n");

#define NEW_LINE(num)                                       \
        for (int i = 0; i < num; ++i)                       \
        {                                                   \
            fprintf(stream, "\n\\newline\n");               \
        }                                                   \
        fprintf(stream, "\n");

#define NEW_PAGE() fprintf(stream, "\n\\newpage\n");

#define START_TASK()                                        \
        tree      = new_node_undef(nullptr);                \
        tree_diff = nullptr;                                \
                                                            \
        new_task         (stream);                          \
        Tree_parsing_main(tree  , get_task());              \
                                                            \
        _$$                                                 \
        Tex_tree         (tree, stream, "f(x)=");           \
        _$$

#define END_TASK()                                          \
        Tree_dtor(tree     );                               \
        Tree_dtor(tree_diff);                               \
                                                            \
        tree      = nullptr;                                \
        tree_diff = nullptr;

#define END()                                               \
        Tree_dtor(tree);                                    \
        Tex_end(stream);

/*____________________________________*/

int main()
{
    START()

//-------------------------------------------------------------------------------------------------------------------------------

    START_TASK()
    Tex_message (stream , "В данном примере перед нами многочлен первой степени. Воспользовавшись правилом взятия производной "
                          "степенной функции, получаем\n");
    _$$
    Tex_tree(tree, stream, "f\'(x)=(", ")\'=");

    tree_diff = diff_main(&tree, "x");
    Tex_tree(tree_diff, stream, nullptr, "=");
    
    Tree_optimize_main(&tree_diff);
    Tex_tree(tree_diff, stream);
    _$$

    Tex_message (stream, "Заметим, что производная константы равна нулю, поэтому если бы вместо цифры 8 в примере №1 стояла "
                         "какая-нибудь другая цифра(например 1, 2, 3, или даже 4), то производная не изменилась бы.\n\n");
    END_TASK()

//-------------------------------------------------------------------------------------------------------------------------------

    START_TASK()
    Tex_message (stream, "Раз уж мы уже научились брать производные многочленов, пример №2 точно не составит труда - "
                         "здесь всё то же самое, за исключением того что нужно немного заняться счётом. Я думаю, на данном "
                         "этапе нашего курса это может сделать даже 1.5-летний ребёнок. Упрощая, получается:\n");
    _$$
    Tex_tree(tree, stream, "f(x)=", "=");
    
    Tree_optimize_main(&tree);
    Tex_tree(tree, stream);
    _$$
    Tex_message (stream, "Осталось только взять производную:\n");
    _$$
    tree_diff = diff_main(&tree, "x");

    Tex_tree(tree     , stream, "f\'(x)=(", ")\'=");
    Tex_tree(tree_diff, stream, nullptr, "=");
    
    Tree_optimize_main(&tree_diff);
    Tex_tree(tree_diff, stream);
    _$$
    END_TASK()

//-------------------------------------------------------------------------------------------------------------------------------

    START_TASK()
    Tex_message(stream, "Давайте немного отвлечёмся от скучного дифференцирования и устного счёта. Для этого предлагаю "
                        "разложить незамысловатую функцию в ряд Тейлора в окрестности -3.\n");
    Tex_message(stream, "Как известно(смотри пример №1), любую функцию f(x) в окрестности точки $x_{0}$ можно округлить до "
                        "$o((x-x_{0})^{n})$ по формуле:\n");
    _$$
    Tex_message(stream, "f(x_{0})=\\sum_{i=0}^{n}\\frac{f^{(i)}(x_{0})x^i}{i!} + o((x-x_{0})^{n})");
    _$$
    Tex_message(stream, "Зафиксируем n = 2. Тогда:\n");

    double k_0 = 0, k_1 = 0, k_2 = 0;

    tree_diff = Teylor(&tree     , stream,   "f(x)=",   "f(-2)=", &k_0);
    
    Tex_message(stream, "Так как $\\pi^{e}<e^{\\pi}$, то:");
    tree      = Teylor(&tree_diff, stream,  "f'(x)=",  "f'(-2)=", &k_1);
    
    Tex_message(stream, "И ежу очевидно, что:");
    tree_diff = Teylor(&tree     , stream, "f''(x)=", "f''(-2)=", &k_2);

    Tex_message(stream, "В итоге, получаем:");
    _$$
    Tex_message(stream, "f(x)="); dump_tex_num(k_0, stream);
    Tex_message(stream, "+"    ); dump_tex_num(k_1, stream);
    Tex_message(stream, "x+"   ); dump_tex_num(k_2, stream);
    Tex_message(stream, "\\frac{x^2}{2}+o((x+3)^{2})");
    _$$

    END_TASK()

//-------------------------------------------------------------------------------------------------------------------------------

    START_TASK()
    
    Tree_plot(tree, nullptr, nullptr, nullptr);

    END()
}

/*_______________________________________________________________________________________________________________________________*/

void header(FILE *const stream)
{
    assert(stream != nullptr);

    Tex_message(stream, "\\begin{center}\n"
                        "{\\bf \\Large\n"
                        "Введение в дифференцирование.\n"
                        "}\n"
                        "\\end{center}\n");
    Tex_message(stream, "Здравствуйте, дорогие школьники, рады вас приветствовать на этом замечательном курсе, который "
                        "даст Вам быстрый старт в спортивное дифференцирование. Этот курс подойдёт всем, в независимости "
                        "от начальных знаний и умений. Разбирая примеры разной сложности шаг за шагом, мы уверены, что "
                        "даже самый неспособный ученик сможет разобраться с материалом.\n\n");
    NEW_PAGE()
}

char *get_task()
{
    sprintf(task, "base/task%d.txt", cur_task);
    ++cur_task;

    return task;
}

void new_task(FILE *const stream)
{
    Tex_message(stream, "{\\bf \\Large\n"
                        "Задача %d\n"
                        "}\n",  cur_task);
}

/*_______________________________________________________________________________________________________________________________*/

extern const char *var_names[];

void Tex_system_vars(Tree_node *system_vars[], FILE *const stream)
{
    assert(system_vars        != nullptr);
    if    (system_vars[ALPHA] == nullptr) return;

    Tex_message(stream, ", где\n");

    for (int i = ALPHA; i < VARS_SIZE && system_vars[i] != nullptr; ++i)
    {
        _$$
        Tex_message(stream, "%s = ", var_names[i]);
        Tex_tree   (system_vars[i], stream);
        _$$
    }
}

Tree_node *Teylor(Tree_node **tree, FILE *const stream, const char *text_f, const char *text_f_point, double *const dbl)
{
    Tree_node *tree_diff = diff_main(tree, "x");

    Tree_node *system_vars[VARS_SIZE] = {};
    Tree_optimize_var_main(tree, system_vars);

    _$$
    Tex_tree       (*tree, stream, text_f);
    _$$
    Tex_system_vars(system_vars, stream);

    *dbl = Tree_get_value_in_point(*tree, system_vars, -2);
    _$$
    Tex_tree   (*tree, stream, text_f_point, "=", system_vars, -2);
    Tex_message(stream, "%lg", *dbl);
    _$$

    Tree_dtor(*tree);
    *tree = nullptr;
    for (int i = 0; i < VARS_SIZE; ++i) Tree_dtor(system_vars[i]);

    return tree_diff;
}

/*_______________________________________________________________________________________________________________________________*/

#define system(s)                   \
        system(s);                  \
        log_message("%s\n", s);

void Tree_plot(Tree_node *root, Tree_node *root_vars[],
               Tree_node *diff, Tree_node *diff_vars[], const double x_min, const double x_max,
                                                        const double y_min, const double y_max)
{
    log_header(__PRETTY_FUNCTION__);

    system("gnuplot");
    //system("set xlabel \"x\"");
    //system("set ylabel \"y\"");
    //system("set grid");

    if (!approx_equal(x_min, POISON)) range_system(x_min, x_max, 'x');
    if (!approx_equal(y_min, POISON)) range_system(y_min, y_max, 'y');

    char root_fmt[BUFF_SIZE] = {};
    char diff_fmt[BUFF_SIZE] = {};

    if (!(func_init(root, root_vars, root_fmt, false) && func_init(diff, diff_vars, diff_fmt, true)))
    {
        log_error("Init is invalid.\n");
        log_end_header();
        return;
    }
    func_system(root_fmt, diff_fmt);

    system("exit");
    log_end_header();
}

bool func_init(Tree_node *root, Tree_node *system_vars[], char *const bracket_fmt, bool is_diff)
{
    if (root == nullptr)
    {
        log_warning("root is nullptr.\n");
        return true;
    }
    if (!Tree_get_bracket_fmt(root, system_vars, bracket_fmt))
    {
        log_error("Tree_get_bracket returned false.\n");
        return false;
    }

    char cmd[2 * BUFF_SIZE] = {};
    if (is_diff) sprintf(cmd, "df(x)=%s", bracket_fmt);
    else         sprintf(cmd,  "f(x)=%s", bracket_fmt);

    system(cmd);
    return true;
}

void func_system(const char *root_fmt, const char *diff_fmt)
{
    assert(root_fmt != nullptr);
    assert(diff_fmt != nullptr);

    if      (root_fmt[0] && diff_fmt[0]) { /*system("plot f(x), df(x)");*/  }
    else if (root_fmt[0])                { /*system("plot f(x)");*/         }
    else if (diff_fmt[0])                { /*system("plot df(x)");*/        }
}

void range_system(const double min_val, const double max_val, const char c)
{
    char    cmd[BUFF_SIZE] = {};
    sprintf(cmd, "set %crange [%lg:%lg]", c, min_val, max_val);
    system (cmd);
}