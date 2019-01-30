#ifndef UTIL_H
#define UTIL_H

#include <stdarg.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct error_t;

#define ERROR_SET(error_ptr, error_code, ...) error_set_(error_ptr, error_code, __LINE__, __FILE__, __func__, __VA_ARGS__)

void error_set_(struct error_t **error_ptr, int error_code, unsigned int line, const char *file, const char *func, const char* msg, ...);

void clear_error(struct error_t **error_ptr);

void print_error_msg(struct error_t *error_ptr, FILE *to);

#endif //UTIL_H