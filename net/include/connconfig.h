#ifndef CONNCONFIG_H
#define CONNCONFIG_H
#include <stdint.h>

enum connection_type{
    local,
    tcp
};

struct connection_config_t{
    const enum connection_type type;
    const uint32_t port;
    const char * const host;
};

struct connection_config_t *allocate_config(enum connection_type, const uint32_t port, const char *host);

void release_config(struct connection_config_t *);

#endif //CONNCONFIG_H