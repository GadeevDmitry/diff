#ifndef ALGORITHM_H
#define ALGORITHM_H

/*_________________________________________FUNCTION_DECLARATIONS_________________________________________*/

bool        approx_equal            (const double a, const double b);
int         get_max                 (int          a, int          b);

void        my_swap                 (void *a, void *b,  const int elem_size);
void       *get_ptr                 (void *begin_array, const int index,     const int elem_size);

void        my_quick_sort           (void        *data, const int elem_size, const int      left, 
                                                                             const int     right,
                                                                             int (*cmp) (void *, void *));

int         is_empty_input_stream   (FILE *const stream);
void        clear_input_stream      (FILE *const stream);

void        get_line                (char *push_in, const int max_size, FILE *const stream);
void        get_word                (char *push_in, const int max_size, FILE *const stream);
void        skip_spaces             (FILE *const stream);

int         is_empty_input_buff     (const char *buff, const int buff_size, int *const pos);
void        clear_input_buff        (const char *buff, const int buff_size, int *const pos);

void        get_line                (char *push_in, const int max_size, const char *buff, const int buff_size, int *const pos);
void        get_word                (char *push_in, const int max_size, const char *buff, const int buff_size, int *const pos);
void        get_word_split          (char *push_in, const int max_size, const char *buff     ,
                                                                        const int   buff_size,
                                                                        int *const  pos      , const char *split);
void        skip_spaces             (                                   const char *buff, const int buff_size, int *const pos);

/*_______________________________________________________________________________________________________*/

#endif //ALGORITHM_H