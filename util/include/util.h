#ifndef UTIL_H
#define UTIL_H

#include <stdarg.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static const int error_desc_max_size = 256;

struct error_t{
    int error_code;
    unsigned int line;
    const char *file;
    const char *func;
    const char *error_desc;
};

#define ERROR_SET(error_ptr, error_code, ...) _error_set(error_ptr, error_code, __LINE__, __FILE__, __func__, __VA_ARGS__)

static inline void _error_set(struct error_t **error_ptr, int error_code, unsigned int line, const char *file, const char *func, const char* msg, ...){
    va_list args;
    va_start(args, msg);
    
    char *error_desc = malloc(error_desc_max_size * sizeof(char));
    //TODO: Handle out of memory

    int bytes_written = vsnprintf(error_desc, error_desc_max_size, msg, args);
    if(bytes_written < 0){
        free(error_desc);
        error_desc = NULL;
    } else if(bytes_written + 1 > error_desc_max_size){
        fprintf(stderr, "WARNING: Error message exceeds the maximum length %d. The rest will be truncated\n", bytes_written);
    }

    struct error_t tmp = {.error_code = error_code, .error_desc = error_desc, .file = file, .line = line, .func = func};
    struct error_t *ptr = malloc(sizeof(*ptr));
    memcpy(ptr, &tmp, sizeof(*ptr));
    *error_ptr = ptr;

    va_end(args);
}
    
static inline void clear_error(struct error_t **error_ptr){
    struct error_t *ptr = *error_ptr;
    const char *error_desc = ptr -> error_desc;
    *error_ptr = NULL;
    free(ptr);
    free(error_desc); //TODO: Fix this discard const qualifier warning
}

static inline void print_error_msg(struct error_t *error_ptr, FILE *to){
    fprintf(to, "Error occurred. Error code: %d, file: %s, line: %d, func: %s. Details: %s\n", 
        error_ptr -> error_code, 
        error_ptr -> file,
        error_ptr -> line,
        error_ptr -> func,
        error_ptr -> error_desc);
}

#endif //UTIL_H