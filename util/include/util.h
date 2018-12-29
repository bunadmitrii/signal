#ifndef UTIL_H
#define UTIL_H

struct error_t{
    const int error_code;
    const char *const error_desc;
    const struct error_t *cause;
};

#endif //UTIL_H