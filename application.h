#ifndef APPLICATION_H
#define APPLICATION_H

#include <stdint.h>

#include "connconfig.h"

enum mode {
    record, 
    play 
};

struct application_config_t{
    struct connection_config_t *conn_config_ptr;
    const enum mode mode;
    const char *const device_name;
    const char *const file_path;
};

int run_application(struct application_config_t);

#endif //APPLICATION_H