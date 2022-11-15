/** @file */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <assert.h>

#include "read_write.h"

bool write_file(const char *file_name, void *data, const int data_size)
{
    FILE  *stream = fopen(file_name, "wb");

    if    (stream == nullptr) return false;
    assert(data   != nullptr);

    fwrite(data, sizeof(char), (size_t) data_size, stream);
    fclose(stream);
    return true;
}

/**
*   @brief Allocates memory for input data and reads it from the file "file_name".
*
*   @param file_name [in]  - name of the file to read from
*   @param size_ptr  [out] - pointer to the size of the file "file_name"
*
*   @return pointer to the array with data and nullptr in case of error
*/

void *read_file(const char *file_name, int *const size_ptr)
{
    assert(file_name != nullptr);
    assert(size_ptr  != nullptr);

    *size_ptr = get_file_size(file_name);

    if (*size_ptr == -1) return nullptr;

    FILE *stream  = fopen(file_name, "r");
    if (  stream == nullptr) return nullptr;

    void *data_ptr = calloc((size_t) *size_ptr, sizeof(char));

    fread (data_ptr, sizeof(char), (size_t) *size_ptr, stream);
    fclose(stream);

    return data_ptr;
}

/**
*   @brief Determines the size (in bytes) of file "file_name".
*
*   @param file_name [in] - name of the file to determine
*
*   @return size (in bytes) of the file
*
*   @note return -1 in case of error
*/

int get_file_size(const char *file_name)
{
    assert(file_name != nullptr);

    struct stat BuffSize = {};

    int StatRet = stat(file_name, &BuffSize);
    if (StatRet == -1) return -1;

    return (int) BuffSize.st_size;
}