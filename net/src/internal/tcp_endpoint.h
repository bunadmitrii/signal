#ifndef TCP_ENDPOINT_INTERNAL_H
#define TCP_ENDPOINT_INTERNAL_H

#include "util.h"
#include "config_internal.h"

struct server_endpoint_t* create_tcp_endpoint_(struct tcp_conf_t, struct error_t**);
void close_tcp_endpoint_(struct server_endpoint_t, struct error_t **);


#endif //TCP_ENDPOINT_INTERNAL_H