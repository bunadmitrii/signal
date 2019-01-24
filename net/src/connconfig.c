#include <malloc.h>
#include <stdlib.h>
#include <string.h>

#include "connconfig.h"

#include "internal/config_internal.h"

struct connection_config_t *allocate_local(const char *local_address){
    struct connection_config_t tmp = {
        .type = local, 
        .conf = {
            .local_conf = {local_address}
        }
    };
    struct connection_config_t *ptr = malloc(sizeof(struct connection_config_t));
    memcpy(ptr, &tmp, sizeof(tmp));
    return ptr;
}

struct connection_config_t *allocate_tcp(const char *hostname, uint16_t port, int backlog){
    struct connection_config_t tmp = {
        .type = tcp, 
        .conf = {
            .tcp_conf = {
                .hostname = hostname, 
                .port = port,
                .backlog = backlog
            }
        }
    };
    struct connection_config_t *ptr = malloc(sizeof(struct connection_config_t));
    memcpy(ptr, &tmp, sizeof(tmp));
    return ptr;
}

void release_config(struct connection_config_t * config_ptr){
    free(config_ptr);
}