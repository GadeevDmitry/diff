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

const char *func      = "F(x)=";
const char *point     = "point=";
const char *degree    = "degree=";

const char *  diff_task = "DIFF:";
const char *taylor_task = "TAYLOR:";
const char *  plot_task = "PLOT:";

static       int    CUR_TASK =          1;
static const char      *file = "math.tex";

const int WIDTH  = 600;
const int HEIGHT = 450;

int main(int argc, const char *argv[])
{
    if (argc != 1)
    {
        fprintf(stderr, "you should give file to parse\n");
        return 0;
    }

    int buff_size = 0;
    const char *buff       = nullptr;
    const char *buff_begin = buff = read_file();
}