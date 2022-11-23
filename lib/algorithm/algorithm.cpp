#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <math.h>

#include "algorithm.h"
#include "../logs/log.h"

/*______________________STATIC_FUNCTION_______________________*/

static bool is_ban_char(const char check_to, const char *ban);

/*____________________________________________________________*/

bool approx_equal(const double a, const double b)
{
    const double delta = 0.0001;
    
    return fabs(a - b) <= delta;
}

long long get_max(long long a, long long b)
{
    if (a > b) return a;
    return b;
}

void my_swap(void *a, void *b, const int elem_size)
{
    assert(a);
    assert(b);

    unsigned char temp = '\0';

    for (int i = 0; i < elem_size; ++i)
    {
        temp = *((unsigned char *)a + i);
               *((unsigned char *)a + i) = *((unsigned char *)b + i);
               *((unsigned char *)b + i) = temp;
    }
}

void *get_ptr(void *begin_array, const int index, const int elem_size)
{
    assert(begin_array);

    void  *ptr = (unsigned char *)begin_array + index * elem_size;
    return ptr;
}

/*__________________________________________ALGORITHM_FUNCTIONS__________________________________________*/

void my_quick_sort(void *data, const int elem_size, const int left, const int right, int (*cmp) (void * elem1, void * elem2))
{
    assert(data);
    assert(cmp );

    if        (left >= right) return;
    int mid = (left  + right) / 2;

    my_swap(get_ptr(data, left, elem_size), get_ptr(data, mid, elem_size), elem_size);

    int cut = left;
    for (int i = left + 1; i <= right; ++i) {

        if ((*cmp)(get_ptr(data, i, elem_size), get_ptr(data, left, elem_size)) <= 0) {

            ++cut;

            my_swap(get_ptr(data, cut, elem_size), get_ptr(data, i, elem_size), elem_size);
        }
    }

    my_swap(get_ptr(data, cut, elem_size), get_ptr(data, left, elem_size), elem_size);

    my_quick_sort(data, elem_size,    left, cut - 1, cmp);
    my_quick_sort(data, elem_size, cut + 1,   right, cmp);
}

/*_________________________________________FUNCTIONS_WITH_STREAM_________________________________________*/

int is_empty_input_stream(FILE *const stream)
{
    assert(stream);

    int     is_empty_temp = ' ';
    while ((is_empty_temp = getc(stream)) == ' ' || is_empty_temp == '\t')
        ;
    ungetc(is_empty_temp, stream);
    
    if (is_empty_temp == '\n' || is_empty_temp == EOF) return 1;

    return 0;
}

void clear_input_stream(FILE *const stream)
{
    assert(stream != NULL);

    int     clear_temp ='#';
    while ((clear_temp = getc(stream)) != '\n' && clear_temp != EOF)
        ;
    ungetc(clear_temp, stream);
}

void get_line(char *push_in, const int max_size, FILE *const stream)
{
    assert(push_in);
    assert(stream );

    skip_spaces(stream);

    int  cnt = 0;
    for (cnt = 0; cnt < max_size - 1; ++cnt)
    {
        int cur_char =  getc(stream);
        if (cur_char == EOF || cur_char == '\n')
        {
            ungetc(cur_char, stream);
            break;
        }
        
        push_in[cnt] = (char) cur_char;
    }
    while (isspace(push_in[cnt - 1]) && cnt > 0)
        --cnt;
    push_in[cnt] = '\0';
}

void get_word(char *push_in, const int max_size, FILE *const stream)
{
    assert(push_in);
    assert(stream );

    skip_spaces(stream);

    int  cnt = 0;
    for (cnt = 0; cnt < max_size - 1; ++cnt)
    {
        int cur_char =  getc(stream);
        if (cur_char == EOF || isspace(cur_char))
        {
            ungetc(cur_char, stream);
            break;
        }

        push_in[cnt] = (char) cur_char;
    }
    push_in[cnt] = '\0';
}

void skip_spaces(FILE *const stream)
{
    assert(stream);

    int cur_char = ' ';
    while (isspace(cur_char = getc(stream)))
        ;
    ungetc(cur_char, stream);
}

/*_________________________________________FUNCTIONS_WITH_BUFFER_________________________________________*/

int is_empty_input_buff(const char *buff, const int buff_size, int *const pos)
{
    assert(buff);
    assert(pos );

    while (*pos < buff_size && (buff[*pos] == ' ' || buff[*pos] == '\t'))
        ++ *pos;

    if (*pos == buff_size || buff[*pos] == '\n' || buff[*pos] == '\0') return 1;

    return 0;
}

void clear_input_buff(const char *buff, const int buff_size, int *const pos)
{
    assert(buff);
    assert(pos );

    while (*pos < buff_size && buff[*pos] != '\n' && buff[*pos] != '\0')
        ++ *pos;
}

void get_line(char *push_in, const int max_size, const char *buff, const int buff_size, int *const pos)
{
    assert(push_in);
    assert(buff   );
    assert(pos    );

    skip_spaces(buff, buff_size, pos);

    int  limit = (buff_size < max_size) ? buff_size : max_size - 1;
    int  cnt = 0;
    for (cnt = 0; cnt < limit; ++cnt)
    {
        if (buff[*pos] == '\0' || buff[*pos] == '\n') break;
        
        push_in[cnt] = buff[*pos];
        ++*pos;
    }
    while (isspace(push_in[cnt - 1]) && cnt > 0)
        --cnt;
    push_in[cnt] = '\0';
}

void get_word(char *push_in, const int max_size, const char *buff, const int buff_size, int *const pos)
{
    assert(push_in);
    assert(buff   );
    assert(pos    );

    skip_spaces(buff, buff_size, pos);

    int limit = (buff_size < max_size) ? buff_size : max_size - 1;
    int  cnt = 0;
    for (cnt = 0; cnt < limit; ++cnt)
    {
        if (buff[*pos] == '\0' || isspace(buff[*pos])) break;

        push_in[cnt] = buff[*pos];
        ++*pos;
    }
    push_in[cnt] = '\0';
}

void get_word_split(char *push_in, const int max_size,  const char *buff     ,
                                                        const int   buff_size,
                                                        int *const  pos      , const char *split)
{
    assert(push_in);
    assert(buff   );
    assert(pos    );

    if (split == nullptr)
    {
        get_word(push_in, max_size, buff, buff_size, pos);
        return;
    }

    skip_spaces(buff, buff_size, pos);

    int limit = (buff_size < max_size) ? buff_size : max_size - 1;
    int  cnt = 0;
    for (cnt = 0; cnt < limit; ++cnt)
    {
        if (is_ban_char(buff[*pos], split) || isspace(buff[*pos])) break;

        push_in[cnt] = buff[*pos];
        ++*pos;
    }
    push_in[cnt] = '\0';
}

static bool is_ban_char(const char check_to, const char *ban)
{
    if (ban == nullptr) return false;

    while (*ban != '\0')
    {
        if (*ban == check_to) return true;
        ++   ban;
    }
    return false;
}

void skip_spaces(const char *buff, const int buff_size, int *const pos)
{
    assert(buff);
    assert(pos );

    while (*pos < buff_size)
    {
        if (!isspace(buff[*pos])) break;
        ++*pos;
    }
}
/*_______________________________________________________________________________________________________*/