#include <malloc.h>
#include <stdlib.h>
#include <string.h>

#include "connconfig.h"

struct connection_config_t *allocate_config(enum connection_type type, const uint32_t port, const char *host){
    struct connection_config_t *config = malloc(sizeof(struct connection_config_t));
    struct connection_config_t tmp = {.type = type, .port = port, .host = host};
    memcpy(config, &tmp, sizeof(tmp));
    return config;
}

void release_config(struct connection_config_t * config_ptr){
    free(config_ptr);
}