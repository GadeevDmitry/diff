#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>

#include "diff.h"
#include "../lib/logs/log.h"
#include "../lib/read_write/read_write.h"
#include "../lib/algorithm/algorithm.h"

static const int   TASK_SIZE =        100;
static const int   VARS_SIZE =        100;
static const int   BUFF_SIZE =      10000;

static       int    CUR_TASK =          1;
static const char      *file = "math.tex";

const int WIDTH  = 600;
const int HEIGHT = 450;

char  task[TASK_SIZE] = {};

FILE *stream = nullptr;

/*____________________________________*/
void    header              ();
char   *get_task            ();
void    new_task            ();
//--------------------------------------
void    skip_line           (const char *buff, const int buff_size, int *const buff_pos);
bool    is_empty_buff       (const char *buff, const int buff_size, int        buff_pos);
//--------------------------------------
bool    task_diff           (const char *filename);
bool    task_optimize_diff  (const char *filename);
bool    task_Taylor         (const char *filename);
bool    task_plot           (const char *filename);
/*____________________________________*/


#define START()                                             \
        Tex_head(file, &stream);                            \
        header();

#define _$$             fprintf(stream, "\n$$\n");
#define NEW_LINE()      fprintf(stream, "\n\\newline\n");
#define NEW_PAGE()      fprintf(stream, "\n\\newpage\n");
#define END()           Tex_end(stream);
#define START_TASK()    new_task();

/*____________________________________*/

int main()
{
    START()

//-------------------------------------------------------------------------------------------------------------------------------

    START_TASK()
    Tex_message (stream , "В данном примере перед нами многочлен первой степени. Воспользовавшись правилом взятия производной "
                          "степенной функции, получаем\n");
    if (!task_diff(get_task()))
    {
        log_error("task_diff failed.\n");
        return 0;
    }

    Tex_message (stream, "Заметим, что производная константы равна нулю, поэтому если бы вместо цифры 8 в примере №1 стояла "
                         "какая-нибудь другая цифра(например 1, 2, 3, или даже 4), то производная не изменилась бы.\n\n");

//-------------------------------------------------------------------------------------------------------------------------------

    START_TASK()
    Tex_message (stream, "Раз уж мы уже научились брать производные многочленов, пример №2 точно не составит труда - "
                         "здесь всё то же самое, за исключением того что нужно немного заняться счётом. Я думаю, на данном "
                         "этапе нашего курса это может сделать даже 1.5-летний ребёнок.\n");
    if (!task_optimize_diff(get_task()))
    {
        log_error("task_optimize_diff failed.\n");
        return 0;
    }
//-------------------------------------------------------------------------------------------------------------------------------
    START_TASK()
    Tex_message(stream, "Давайте немного отвлечёмся от скучного дифференцирования и устного счёта. Для этого предлагаю "
                        "разложить незамысловатую функцию в ряд Тейлора в окрестности -3.\n");
    Tex_message(stream, "Как известно(смотри пример №1), любую функцию f(x) в окрестности точки $x_{0}$ можно округлить до "
                        "$o((x-x_{0})^{n})$ по формуле:\n");
    _$$
    Tex_message(stream, "f(x_{0})=\\sum_{i=0}^{n}\\frac{f^{(i)}(x_{0})x^i}{i!} + o((x-x_{0})^{n})");
    _$$
    if (!task_Taylor(get_task()))
    {
        log_error("Task_Taylor failed.\n");
        return 0;
    }
//-------------------------------------------------------------------------------------------------------------------------------
    START_TASK()
    Tex_message(stream, "Геометрический смысл производной.");
    if (!task_plot(get_task()))
    {
        log_error("task_plot failed.\n");
        return 0;
    }

//-------------------------------------------------------------------------------------------------------------------------------

    END()
}

/*_______________________________________________________________________________________________________________________________*/

void header()
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

void new_task()
{
    Tex_message(stream, "{\\bf \\Large\n"
                        "Задача %d\n"
                        "}\n",  CUR_TASK);
}

/*_______________________________________________________________________________________________________________________________*/

#define tree_check(tree)                            \
    if (tree == nullptr)                            \
    {                                               \
        log_free (buff);                            \
        log_error("Parsing failed.\n");             \
        return false;                               \
    }

#define fmt_check(buff, buff_size, buff_pos)        \
    if (!is_empty_buff(buff, buff_size, buff_pos))  \
    {                                               \
        Tree_dtor(tree);                            \
        log_free (buff);                            \
        log_error("Wrong format.\n");               \
        return false;                               \
    }

bool task_diff(const char *filename)
{
    assert(filename != nullptr);

    int   buff_pos  = 0;
    int   buff_size = 0;
    char *buff      = (char *) read_file(filename, &buff_size);
    
    Tree_node *tree = nullptr;
    Tree_node *diff = nullptr;
    
    tree = Tree_parsing_buff(buff);
    tree_check(tree);

    skip_line(buff, buff_size, &buff_pos);
    fmt_check(buff, buff_size,  buff_pos);

    _$$
    Tex_tree(tree, stream, "f(x)=");
    _$$
    _$$
    Tex_tree(tree, stream, "f\'(x)=(", ")\'=");

    diff = diff_main(&tree, nullptr, "x");
    Tex_tree(diff, stream,  nullptr, "=");
    
    Tree_optimize_main(&diff);
    Tex_tree(diff, stream);
    _$$

    Tree_dtor(tree);
    Tree_dtor(diff);
    log_free (buff);
    return true;
}
/*_______________________________________________________________________________________________________________________________*/
bool task_optimize_diff(const char *filename)
{
    assert(filename != nullptr);

    int   buff_pos  = 0;
    int   buff_size = 0;
    char *buff      = (char *) read_file(filename, &buff_size);

    Tree_node *tree = nullptr;
    Tree_node *diff = nullptr;

    tree = Tree_parsing_buff(buff);
    tree_check(tree);

    skip_line(buff, buff_size, &buff_pos);
    fmt_check(buff, buff_size,  buff_pos);

    _$$
    Tex_tree(tree, stream, "f(x)=");
    _$$

    Tex_message(stream, "Упростив, получаем.\n");
    _$$
    Tex_tree(tree, stream, "f(x)=", "=");
    
    Tree_optimize_main(&tree);
    Tex_tree(tree, stream);
    _$$
    Tex_message(stream, "Производная:");
    _$$
    diff = diff_main(&tree, nullptr, "x");

    Tex_tree(tree, stream, "f\'(x)=(", ")\'=");
    Tex_tree(diff, stream, nullptr, "=");
    
    Tree_optimize_main(&diff);
    Tex_tree(diff, stream);
    _$$

    Tree_dtor(tree);
    Tree_dtor(diff);
    log_free (buff);
    return true;
}
/*_______________________________________________________________________________________________________________________________*/
Tree_node  *Taylor_iter     (Tree_node **tree, const char *text_func, const char *text_func_point,  const double point,
                                                                                                    double *const  val);
void        Taylor_print    (const double *factors, const int Taylor_rate);

bool        get_dbl         (const char *buff, const int buff_size, int *const buff_pos, double *const ret);
bool        get_int         (const char *buff, const int buff_size, int *const buff_pos, int *const ret);

void        Tex_system_vars (Tree_node *system_vars[]);
//--------------------------------------

bool task_Taylor(const char *filename)
{
    log_header(__PRETTY_FUNCTION__);

    assert(filename != nullptr);

    int   buff_pos  = 0;
    int   buff_size = 0;
    char *buff      = (char *) read_file(filename, &buff_size);

    Tree_node *tree = Tree_parsing_buff(buff);
    tree_check(tree);

    skip_line(buff, buff_size, &buff_pos);
    
    double Taylor_point = 0;
    int    Taylor_rate  = 0;

    if (!get_dbl(buff, buff_size, &buff_pos, &Taylor_point))
    {
        Tree_dtor(tree);
        log_free (buff);
        log_error("Point to explore is invalid.\n"); return false;
    }
    if (!get_int(buff, buff_size, &buff_pos, &Taylor_rate )){
        Tree_dtor(tree);
        log_free (buff);
        log_error("Rate is invalid.\n");
        return false;
    }
    fmt_check(buff, buff_size,  buff_pos);

    _$$
    Tex_tree(tree, stream, "f(x)=");
    _$$

    double factors[Taylor_rate + 1] = {};

    for (int i = 0; i <= Taylor_rate; ++i)
    {
        char text_func      [TASK_SIZE] = {};
        char text_func_point[TASK_SIZE] = {};

        sprintf(text_func      , "f^{(%d)}(x)=", i);
        sprintf(text_func_point, "f^{(%d)}(%lg)=", i, Taylor_point);

        tree = Taylor_iter(&tree, text_func, text_func_point, Taylor_point, &factors[i]);
    }

    Taylor_print(factors, Taylor_rate);
    Tree_dtor   (tree);
    log_free    (buff);

    log_end_header();
    return true;
}

void Taylor_print(const double *factors, const int Taylor_rate)
{
    assert(factors != nullptr);

    _$$
    Tex_message(stream, "f(x)=");

    for (int i = 0; i <= Taylor_rate; ++i)
    {
        if (i) Tex_message(stream, "+");

        dump_tex_num(factors[i], stream);
        Tex_message(stream, "\\frac{x^%d}{%d!}", i, i);
    }
    _$$
}

Tree_node *Taylor_iter(Tree_node **tree, const char *text_func, const char *text_func_point, const double point, double *const val)
{
    assert( tree != nullptr);
    assert(*tree != nullptr);

    assert(val   != nullptr);

    Tree_node *diff = diff_main(tree, nullptr, "x");

    Tree_node *system_vars[VARS_SIZE] = {};
    Tree_optimize_var_main(tree, system_vars, VARS_SIZE);

    _$$
    Tex_tree(*tree, stream, text_func);
    if (system_vars != nullptr && system_vars[0] != nullptr)
    {
        Tex_message(stream, ",");
        _$$
        Tex_system_vars(system_vars);
    }
    else { _$$ }

    *val = Tree_get_value_in_point(*tree, system_vars, point);

    _$$
    Tex_tree   (*tree, stream, text_func_point, "=", system_vars, point);
    Tex_message(stream, "%lg", *val);
    _$$

    Tree_dtor(*tree);
    *tree = nullptr;
    for (int i = 0; i < VARS_SIZE; ++i) Tree_dtor(system_vars[i]);

    return diff;
}

void Tex_system_vars(Tree_node *system_vars[])
{
    assert(system_vars    != nullptr);
    if    (system_vars[0] == nullptr) return;

    Tex_message(stream, "где");

    for (int i = 0; i < VARS_SIZE && system_vars[i] != nullptr; ++i)
    {
        _$$
        Tex_message(stream, "x_{%d} = ", i);
        Tex_tree   (system_vars[i], stream);
        _$$
    }
}
/*_______________________________________________________________________________________________________________________________*/
void print_tree_diff_tangent(Tree_node *tree   , Tree_node *tree_vars   [],
                             Tree_node *diff   , Tree_node *diff_vars   [],
                             Tree_node *tangent, Tree_node *tangent_vars[], const double point);

Tree_node  *get_tangent     (Tree_node *tree   , Tree_node *   tree_vars[],
                             Tree_node *diff   , Tree_node *   diff_vars[], const double point);
void        make_plot       (Tree_node *tree   , Tree_node *   tree_vars[],
                             Tree_node *tangent, Tree_node *tangent_vars[], const double x_min, const double x_max);
//--------------------------------------
bool task_plot(const char *filename)
{
    log_header(__PRETTY_FUNCTION__);

    assert(filename != nullptr);

    int   buff_size = 0;
    int   buff_pos  = 0;
    char *buff      = (char *) read_file(filename, &buff_size);

    Tree_node *tree                 = nullptr;
    Tree_node *tree_vars[VARS_SIZE] = {};

    Tree_node *diff                 = nullptr;
    Tree_node *diff_vars[VARS_SIZE] = {};

    Tree_node *tangent                 = nullptr;
    Tree_node *tangent_vars[VARS_SIZE] = {};

    tree = Tree_parsing_buff(buff);
    tree_check(tree);

    _$$
    Tex_tree(tree, stream, "f(x)=");
    _$$

    skip_line(buff, buff_size, &buff_pos);
    
    double point = 0;
    if (!get_dbl(buff, buff_size, &buff_pos, &point))
    {
        Tree_dtor(tree);
        log_free (buff);
        log_error("Point to explore is invalid.\n");
        return false;
    }
    fmt_check   (buff, buff_size,  buff_pos);

    diff    = diff_main  (&tree, tree_vars, "x");
    tangent = get_tangent( tree, tree_vars, diff, diff_vars, point);
    
    Tree_optimize_var_main(&tree   ,    tree_vars, VARS_SIZE);
    Tree_optimize_var_main(&diff   ,    diff_vars, VARS_SIZE);
    Tree_optimize_var_main(&tangent, tangent_vars, VARS_SIZE);

    print_tree_diff_tangent(tree, tree_vars, diff, diff_vars, tangent, tangent_vars, point);

    make_plot(tree, tree_vars, tangent, tangent_vars, point - 2, point + 2);

    Tree_dtor(tree);
    Tree_dtor(diff);
    Tree_dtor(tangent);
    for (int i = 0; i < VARS_SIZE; ++i)
    {
        Tree_dtor(   tree_vars[i]);
        Tree_dtor(   diff_vars[i]);
        Tree_dtor(tangent_vars[i]);
    }
    log_free(buff);
    return true;
}

void print_tree_diff_tangent(Tree_node *tree   , Tree_node *tree_vars   [],
                             Tree_node *diff   , Tree_node *diff_vars   [],
                             Tree_node *tangent, Tree_node *tangent_vars[], const double point)
{
    assert(tree    != nullptr);
    assert(diff    != nullptr);
    assert(tangent != nullptr);

    Tex_message(stream, "Функция:\n");
    _$$
    Tex_tree(tree, stream, "f(x)=", nullptr, tree_vars);
    if (tree_vars != nullptr && tree_vars[0] != nullptr) { Tex_message(stream, ","); _$$ Tex_system_vars(tree_vars); }
    else { _$$ }

    Tex_message(stream, "Производная:\n");
    _$$
    Tex_tree(diff, stream, "f'(x)=", nullptr, diff_vars);
    if (diff_vars != nullptr && diff_vars[0] != nullptr) { Tex_message(stream, ","); _$$ Tex_system_vars(diff_vars); }
    else { _$$ }

    Tex_message(stream, "Касательная в точке %lg:", point);
    _$$
    Tex_tree(tangent, stream, "g(x)=", nullptr, tangent_vars);
    if (tangent_vars != nullptr && tangent_vars[0] != nullptr) { Tex_message(stream, ","); _$$ Tex_system_vars(tangent_vars); }
    else { _$$ }
}

Tree_node *get_tangent(Tree_node *tree, Tree_node *tree_vars[], Tree_node *diff, Tree_node *diff_vars[], const double point)
{
    assert(tree != nullptr);
    assert(diff != nullptr);

    double f_val = Tree_get_value_in_point(tree, tree_vars, point);
    double slant = Tree_get_value_in_point(diff, diff_vars, point);

    char tangent_buff[TASK_SIZE] = {};
    sprintf(tangent_buff, "%lg+(%lg)*(x-%lg)\n", f_val, slant, point);

    return Tree_parsing_buff(tangent_buff);
}
//--------------------------------------
bool func_init      (Tree_node  *root, Tree_node *system_vars[], char *const bracket_fmt, bool is_tangent,  char      *cmd,
                                                                                                            int *const cmd_pos);

void func_system    (const char *root_fmt, const char *diff_fmt,                    char *cmd, int *const cmd_pos);
void range_system   (const double min_val, const double max_val, const char axis,   char *cmd, int *const cmd_pos);
void print_buff     (char *buff, int *const buff_pos, const char *message);
void settings       (char *cmd , int *const cmd_pos , const char *file_dump);
//--------------------------------------
void make_plot(Tree_node *tree   , Tree_node *   tree_vars[],
               Tree_node *tangent, Tree_node *tangent_vars[], const double x_min, const double x_max)
{
    static int cur_png = 0;

    char    dump_file[BUFF_SIZE] = {};
    sprintf(dump_file, "math_png/image%d.png", cur_png);
    
    FILE *output = fopen(dump_file, "w");

    char cmd[BUFF_SIZE] = {};
    int  cmd_pos = 0;

    settings(cmd, &cmd_pos, dump_file);
    range_system(x_min, x_max, 'x', cmd, &cmd_pos);

    char    tree_fmt[BUFF_SIZE] = {};
    char tangent_fmt[BUFF_SIZE] = {};

    if (!(func_init(tree,       tree_vars,    tree_fmt, false, cmd, &cmd_pos) &&
          func_init(tangent, tangent_vars, tangent_fmt,  true, cmd, &cmd_pos)))
    {
        log_error("Init is invalid.\n");
        return;
    }
    func_system(tree_fmt, tangent_fmt, cmd, &cmd_pos);

    print_buff(cmd, &cmd_pos, "exit\"");
    system(cmd);
    fclose(output);

    Tex_message(stream, "\\begin{figure}[h]\n"
                        "\\centering\n"
                        "\\includegraphics[width=0.5\\linewidth]{%s}\n"
                        "\\end{figure}", dump_file);
}

bool func_init(Tree_node *root, Tree_node *system_vars[], char *const bracket_fmt, bool is_tangent, char *cmd, int *const cmd_pos)
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
    if (is_tangent) sprintf(init, "g(x)=%s;", bracket_fmt);
    else            sprintf(init, "f(x)=%s;", bracket_fmt);

    print_buff(cmd, cmd_pos, init);
    return true;
}

void func_system(const char *root_fmt, const char *diff_fmt, char *cmd, int *const cmd_pos)
{
    assert(root_fmt != nullptr);
    assert(diff_fmt != nullptr);

    if      (root_fmt[0] && diff_fmt[0]) { print_buff(cmd, cmd_pos, "plot f(x), g(x);"); }
    else if (root_fmt[0])                { print_buff(cmd, cmd_pos, "plot f(x);");       }
    else if (diff_fmt[0])                { print_buff(cmd, cmd_pos, "plot g(x);");       }
}

void range_system(const double min_val, const double max_val, const char axis, char *cmd, int *const cmd_pos)
{
    assert(cmd     != nullptr);
    assert(cmd_pos != nullptr);

    char    set[BUFF_SIZE] = {};
    sprintf(set, "set %crange [%lg:%lg];", axis, min_val, max_val);

    print_buff(cmd, cmd_pos, set);
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

void settings(char *cmd, int *const cmd_pos, const char *file_dump)
{
    assert(cmd       != nullptr);
    assert(cmd_pos   != nullptr);   
    assert(file_dump != nullptr);

    char buff[BUFF_SIZE] = {};

    sprintf(buff,   "gnuplot -persist -e \""
                    "set xlabel \\\"x\\\";"
                    "set ylabel \\\"y\\\";"
                    "set terminal png size %d, %d;"
                    "set output \\\"%s\\\";"
                    "set grid;", WIDTH, HEIGHT, file_dump);

    print_buff(cmd, cmd_pos, buff);
}
/*_______________________________________________________________________________________________________________________________*/
void skip_line(const char *buff, const int buff_size, int *const buff_pos)
{
    assert(buff != nullptr);

    while (*buff_pos < buff_size && buff[*buff_pos] != '\n' && buff[*buff_pos] != '\0') ++(*buff_pos);

    if (*buff_pos < buff_size && buff[*buff_pos] == '\n') ++(*buff_pos);
}

bool is_empty_buff(const char *buff, const int buff_size, int buff_pos)
{
    assert(buff != nullptr);

    while (buff_pos < buff_size)
    {
        if (!isspace(buff[buff_pos])) { return false; }
        ++buff_pos;
    }
    return true;
}

bool get_dbl(const char *buff, const int buff_size, int *const buff_pos, double *const ret)
{
    assert(buff     != nullptr);
    assert(buff_pos != nullptr);
    assert(ret      != nullptr);

    if (*buff_pos > buff_size) return false;

    int num_len = 0;
    if (sscanf(buff + *buff_pos, "%lg%n", ret, &num_len) != 1) return false;

    *buff_pos += num_len;
    return true;
}

bool get_int(const char *buff, const int buff_size, int *const buff_pos, int *const ret)
{
    assert(buff     != nullptr);
    assert(buff_pos != nullptr);
    assert(ret      != nullptr);

    if (*buff_pos > buff_size) return false;

    int num_len = 0;
    if (sscanf(buff + *buff_pos, "%d%n", ret, &num_len) != 1) return false;

    *buff_pos += num_len;
    return true;
}