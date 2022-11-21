#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>

#include "diff.h"
#include "../lib/logs/log.h"

static const int   TASK_SIZE =        100;
static       int    cur_task =          1;
static const char      *file = "math.tex";

char  task[TASK_SIZE] = {};

/*____________________________________*/

void  header    (FILE *const stream);
void  new_task  (FILE *const stream);
char *get_task  ();

/*____________________________________*/


#define START()                                             \
        FILE      *stream    = nullptr;                     \
        Tree_node *tree      = nullptr;                     \
        Tree_node *tree_diff = nullptr;                     \
                                                            \
        Tex_head(file, &stream);                            \
        header         (stream);

#define _$$      fprintf(stream, "\n$$\n");
#define NEW_PAGE fprintf(stream, "\n\n\\newpage\n\n");

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

    tree_diff = diff_main(&tree);
    Tex_tree(tree_diff, stream, nullptr, "=");
    
    Tree_optimize_main(&tree_diff);
    Tex_tree(tree_diff, stream);
    _$$

    Tex_message (stream, "Заметим, что производная константы равна нулю, поэтому если бы вместо цифры 8 в примере №1 стояла "
                         "какая-нибудь другая цифра(например 1, 2, 3 или даже 4), то производная не изменилась бы.\n\n");
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
    tree_diff = diff_main(&tree);

    Tex_tree(tree     , stream, "f\'(x)=(", ")\'=");
    Tex_tree(tree_diff, stream, nullptr, "=");
    
    Tree_optimize_main(&tree_diff);
    Tex_tree(tree_diff, stream);
    _$$
    END_TASK()

//-------------------------------------------------------------------------------------------------------------------------------

    NEW_PAGE
    START_TASK()
    Tex_message(stream, "Давайте немного отвлечёмся от скучного дифференцирования и устного счёта. Для этого предлагаю "
                        "разложить незамысловатую функцию в ряд Тейлора в окрестности -3.\n");
    _$$
    tree_diff = diff_main(&tree);

    Tex_tree(tree     , stream, "f\'(x)=(", ")\'=");
    Tex_tree(tree_diff, stream, nullptr, "=");

    Tree_optimize_main(&tree_diff);
    Tex_tree(tree_diff, stream);
    _$$
    END_TASK()

//-------------------------------------------------------------------------------------------------------------------------------

    END()
}

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
                        "даже самый неспособный ученик сможет разобраться с материалом.\n\n"
                        "\\newpage\n\n");
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