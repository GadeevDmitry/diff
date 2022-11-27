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
static const int   BUFF_SIZE =      10000;

static       int    CUR_TASK =          1;
static const char      *file = "math.tex";

const int WIDTH  = 600;
const int HEIGHT = 450;

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
                     Tree_node *diff, Tree_node *diff_vars[], FILE *const stream,
                                                              const char  *label = nullptr, const double x_min = POISON,
                                                                                            const double x_max = POISON,
                                                                                            const double y_min = POISON,
                                                                                            const double y_max = POISON);

bool func_init      (Tree_node *root, Tree_node *system_vars[], char *const bracket_fmt, bool is_diff,  char      *buff,
                                                                                                        int *const buff_pos);
void func_system    (const char *root_fmt, const char *diff_fmt,                                        char      *buff,
                                                                                                        int *const buff_pos);
void range_system   (const double min_val, const double max_val, const char c,                          char      *buff,
                                                                                                        int *const buff_pos);
void print_buff     (char *buff, int *const buff_pos, const char *message );
void settings       (char *cmd , int *const  cmd_pos, const char *filename);

/*____________________________________*/


#define START()                                             \
        FILE      *stream    = nullptr;                     \
        Tree_node *tree      = nullptr;                     \
        Tree_node *tree_diff = nullptr;                     \
                                                            \
        Tex_head(file, &stream);                            \
        header         (stream);

#define _$$     fprintf(stream, "\n$$\n");

#define NEW_LINE(num)                                       \
        for (int i = 0; i < num; ++i)                       \
        {                                                   \
            fprintf(stream, "\n\\newline\n");               \
        }                                                   \
        fprintf(stream, "\n");

#define NEW_PAGE() fprintf(stream, "\n\\newpage\n");

#define START_TASK()                                        \
        new_task(stream);                                   \
        tree = Tree_parsing_main(get_task());               \
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

#define END() Tex_end(stream);

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

    tree_diff = diff_main(&tree, nullptr, "x");
    Tex_tree(tree_diff, stream,  nullptr, "=");
    
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
    tree_diff = diff_main(&tree, nullptr, "x");

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

    tree_diff = Teylor(&tree     , stream,   "f(x)=",  "f(-2)=",  &k_0);
    
    Tex_message(stream, "Так как $\\pi^{e}<e^{\\pi}$, то:");
    tree      = Teylor(&tree_diff, stream,  "f'(x)=",  "f'(-2)=", &k_1);
    
    Tex_message(stream, "И ежу понятно, что:");
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
    Tex_message(stream, "Геометрический смысл производной.(Задача взята из вступительных экзаменов в первый класс "
                        "среди китайских школьников). ");
    Tex_message(stream, "Для того чтобы понять, в чём же смысл(геометрический) производной,\n\n"
                        "1)Найдём производную:\n");

    Tree_node *system_vars[VARS_SIZE] = {};
    tree_diff =  diff_main(&tree, nullptr, "x");
    Tree_optimize_var_main(&tree_diff, system_vars, VARS_SIZE);
    _$$
    Tex_tree(tree_diff, stream, "f'(x)=", nullptr, system_vars);
    _$$
    Tex_system_vars(system_vars, stream);

    Tex_message(stream, "2)Вычислим значение производной в произволтной точке, например в x=3:");
    
    double  f_point = Tree_get_value_in_point(tree     , nullptr    , 3);
    double df_point = Tree_get_value_in_point(tree_diff, system_vars, 3);
    _$$
    Tex_tree(tree_diff, stream, "f'(3)=", "=", system_vars, 3);
    Tex_message(stream, "%lg", df_point);
    _$$

    Tex_message(stream, "3)Построим прямую с угловым коэффициентом, равным значению производной в точке 3, "
                        "проходящую через точку 3.\n\n");
    
    char tangent_buff[BUFF_SIZE] = {};
    sprintf(tangent_buff, "%lg+%lg*(x-3)\n", f_point, df_point);
    Tree_node *tangent = Tree_parsing_buff(tangent_buff);

    Tree_plot(tree, nullptr, tangent, nullptr, stream, "f(x) и касательная в точке x=3", 0, 5, 1, 7);

    Tex_message(stream, "4)Заметим, что прямая, построенная нами, оказалась касательной к графику. Следовательно, "
                        "геометрический смысл производной - угловой коэффициент касательной к графику.\n");
    
    for (int i = 0; i < VARS_SIZE; ++i) Tree_dtor(system_vars[i]);
    Tree_dtor(tangent);
    END_TASK()

//-------------------------------------------------------------------------------------------------------------------------------

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
                        "даст Вам быстрый старт в дифференцирование. Этот курс подойдёт всем, в независимости "
                        "от начальных знаний и умений. Разбирая примеры разной сложности шаг за шагом, мы уверены, что "
                        "даже самый неспособный ученик сможет разобраться с материалом.\n\n");
    NEW_PAGE()
}

char *get_task()
{
    sprintf(task, "base/task%d.txt", CUR_TASK);
    ++CUR_TASK;

    return task;
}

void new_task(FILE *const stream)
{
    Tex_message(stream, "{\\bf \\Large\n"
                        "Задача %d\n"
                        "}\n",  CUR_TASK);
}

/*_______________________________________________________________________________________________________________________________*/

extern const char *var_names[];

void Tex_system_vars(Tree_node *system_vars[], FILE *const stream)
{
    assert(system_vars    != nullptr);
    if    (system_vars[0] == nullptr) return;

    Tex_message(stream, ", где\n");

    for (int i = 0; i < VARS_SIZE && system_vars[i] != nullptr; ++i)
    {
        _$$
        Tex_message(stream, "x_{%d} = ", i);
        Tex_tree   (system_vars[i], stream);
        _$$
    }
}

Tree_node *Teylor(Tree_node **tree, FILE *const stream, const char *text_f, const char *text_f_point, double *const dbl)
{
    assert( tree != nullptr);
    assert(*tree != nullptr);

    Tree_node *tree_diff = diff_main(tree, nullptr, "x");

    Tree_node *system_vars[VARS_SIZE] = {};
    Tree_optimize_var_main(tree, system_vars, VARS_SIZE);

    _$$
    Tex_tree(*tree, stream, text_f);
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

#define print_cmd(s) print_buff(cmd, cmd_pos, s)

void Tree_plot(Tree_node *root, Tree_node *root_vars[],
               Tree_node *diff, Tree_node *diff_vars[], FILE *const stream,
                                                        const char  *label, const double x_min, const double x_max,
                                                                            const double y_min, const double y_max)
{
    log_header(__PRETTY_FUNCTION__);

    static int cur_png = 0;

    char filename[BUFF_SIZE] = {};
    sprintf(filename, "math_png/image%d.png", cur_png);;
    FILE *output = fopen(filename, "w");

    char cmd[BUFF_SIZE] = {};
    int cmd_pos = 0;

    settings(cmd, &cmd_pos, filename);

    if (!approx_equal(x_min, POISON)) range_system(x_min, x_max, 'x', cmd, &cmd_pos);
    if (!approx_equal(y_min, POISON)) range_system(y_min, y_max, 'y', cmd, &cmd_pos);

    char root_fmt[BUFF_SIZE] = {};
    char diff_fmt[BUFF_SIZE] = {};

    if (!(func_init(root, root_vars, root_fmt, false, cmd, &cmd_pos) && func_init(diff, diff_vars, diff_fmt, true, cmd, &cmd_pos)))
    {
        log_error("Init is invalid.\n");
        log_end_header();
        return;
    }
    func_system(root_fmt, diff_fmt, cmd, &cmd_pos);

    print_buff(cmd, &cmd_pos, "exit\"");
    system(cmd);
    fclose(output);

    if (label != nullptr)
    {
        Tex_message(stream, "\\begin{figure}[h]\n"
                            "\\includegraphics[width=0.5\\linewidth]{%s}\n"
                            "\\caption{%s}\n"
                            "\\end{figure}", filename, label);
    }
    else
    {
        Tex_message(stream, "\\begin{figure}[h]\n"
                            "\\centering\n"
                            "\\includegraphics[width=0.5\\linewidth]{%s}\n"
                            "\\end{figure}", filename);
    }
    log_end_header();
}

bool func_init(Tree_node *root, Tree_node *system_vars[], char *const bracket_fmt, bool is_diff, char *cmd, int *const cmd_pos)
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

    char init[2 * BUFF_SIZE] = {};
    if (is_diff) sprintf(init, "df(x)=%s;", bracket_fmt);
    else         sprintf(init,  "f(x)=%s;", bracket_fmt);

    print_cmd(init);
    return true;
}

void func_system(const char *root_fmt, const char *diff_fmt, char *cmd, int *const cmd_pos)
{
    assert(root_fmt != nullptr);
    assert(diff_fmt != nullptr);

    if      (root_fmt[0] && diff_fmt[0]) { print_cmd("plot f(x), df(x);"); }
    else if (root_fmt[0])                { print_cmd("plot f(x);");        }
    else if (diff_fmt[0])                { print_cmd("plot df(x);");       }
}

void range_system(const double min_val, const double max_val, const char c, char *cmd, int *const cmd_pos)
{
    assert(cmd     != nullptr);
    assert(cmd_pos != nullptr);

    char    set[BUFF_SIZE] = {};
    sprintf(set, "set %crange [%lg:%lg];", c, min_val, max_val);

    print_cmd(set);
}

void print_buff(char *buff, int *const buff_pos, const char *message)
{
    assert(buff     != nullptr);
    assert(buff_pos != nullptr);
    assert(message  != nullptr);

    int added = 0;
    sprintf(buff + *buff_pos, "%s%n", message, &added);
    *buff_pos += added;
}

void settings(char *cmd, int *const cmd_pos, const char *filename)
{
    assert(cmd      != nullptr);
    assert(cmd_pos  != nullptr);   
    assert(filename != nullptr);

    char buff[BUFF_SIZE] = {};

    sprintf(buff,   "gnuplot -persist -e \""
                    "set xlabel \\\"x\\\";"
                    "set ylabel \\\"y\\\";"
                    "set terminal png size %d, %d;"
                    "set output \\\"%s\\\";"
                    "set grid;", WIDTH, HEIGHT, filename);
    print_cmd(buff);
}
