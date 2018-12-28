#ifndef NET_SERVER_ENDPOINT_INTERNAL_H
#define NET_SERVER_ENDPOINT_INTERNAL_H

#include "op_result.h"
#include "config_internal.h"

struct server_endpoint_t* _craete_local_endpoint(struct local_conf_t local_conf_ptr);
enum net_op_result _close_local_endpoint(const struct local_conf_t, int);

#endif //NET_SERVER_ENDPOINT_INTERNAL_H