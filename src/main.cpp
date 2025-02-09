#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>

#include "diff.h"
#include "../lib/logs/log.h"

int main()
{
    Tree_node *root = Tree_parsing_main("base/example.txt");
    Tree_dump_graphviz(root);

    Tree_node *cproot = tree_copy(root);
    Tree_dump_graphviz(cproot);

    Tree_dtor(root);
    Tree_dtor(cproot);

}