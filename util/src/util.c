#include "util.h"

#define ERROR_DESC_MAX_SIZE 256

struct error_t{
    int error_code;
    unsigned int line;
    const char *file;
    const char *func;
    const char *error_desc;
};

void error_set_(struct error_t **error_ptr, int error_code, unsigned int line, const char *file, const char *func, const char* msg, ...){
    va_list args;
    va_start(args, msg);
    
    char *error_desc = malloc(ERROR_DESC_MAX_SIZE * sizeof(char));
    //TODO: Handle out of memory

    int bytes_written = vsnprintf(error_desc, ERROR_DESC_MAX_SIZE, msg, args);
    if(bytes_written < 0){
        free(error_desc);
        error_desc = NULL;
    } else if(bytes_written + 1 > ERROR_DESC_MAX_SIZE){
        fprintf(stderr, "WARNING: Error message exceeds the maximum length %d. The rest will be truncated\n", bytes_written);
    }

    struct error_t tmp = {.error_code = error_code, .error_desc = error_desc, .file = file, .line = line, .func = func};
    struct error_t *ptr = malloc(sizeof(*ptr));
    memcpy(ptr, &tmp, sizeof(*ptr));
    *error_ptr = ptr;

    va_end(args);
}
 
void clear_error(struct error_t **error_ptr){
    struct error_t *ptr = *error_ptr;
    const char *error_desc = ptr -> error_desc;
    *error_ptr = NULL;
    free(ptr);
    free((char *) error_desc);
}

void print_error_msg(struct error_t *error_ptr, FILE *to){
    fprintf(to, "Error occurred. Error code: %d, file: %s, line: %d, func: %s. Details: %s\n", 
        error_ptr -> error_code, 
        error_ptr -> file,
        error_ptr -> line,
        error_ptr -> func,
        error_ptr -> error_desc);
}