#ifndef NET_SERVER_ENDPOINT_INTERNAL_H
#define NET_SERVER_ENDPOINT_INTERNAL_H

#include "util.h"
#include "config_internal.h"

struct server_endpoint_t* _create_local_endpoint(struct local_conf_t local_conf_ptr, struct error_t**);
void _close_local_endpoint(const struct local_conf_t, int, struct error_t**);

#endif //NET_SERVER_ENDPOINT_INTERNAL_H