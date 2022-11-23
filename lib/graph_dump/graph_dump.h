#ifndef GRAPH_DUMP_H
#define GRAPH_DUMP_H

static const int   GRAPHVIZ_SIZE_FILE = 100;
static const int   GRAPHVIZ_SIZE_CMD  = 300;

static const char *graphviz_color_names[]   =
{
    "lightblue"     ,
    "lightgreen"    ,
    "lightgrey"     ,
    "darkorange1"   ,
    "deeppink"      ,

    "darkblue"      ,
    "darkred"       ,
    "darkgreen"     ,
    "darkorange3"   ,
    "deeppink3"     ,

    "gold"          ,
    "yellow"        ,
    "white"         ,
    "black"
};

enum GRAPHVIZ_COLOR
{
    LIGHT_BLUE      ,
    LIGHT_GREEN     ,
    LIGHT_GREY      ,
    LIGHT_ORANGE    ,
    LIGHT_PINK      ,

    DARK_BLUE       ,
    DARK_RED        ,
    DARK_GREEN      ,
    DARK_ORANGE     ,
    DARK_PINK       ,

    GOLD            ,
    YELLOW_HTML     ,
    WHITE           ,
    BLACK           ,
};

#endif //GRAPH_DUMP_H