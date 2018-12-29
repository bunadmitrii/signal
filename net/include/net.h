#ifndef NET_H
#define NET_H

#include <unistd.h>

#include "connconfig.h"
#include "connection.h"
#include "op_result.h"
#include "util.h"

enum net_op_result initialize_server_endpoint(struct server_endpoint_t**, struct connection_config_t *, struct error_t **);

enum net_op_result release_server_endpoing(const struct server_endpoint_t*, struct error_t **);

enum net_op_result await_connection(const struct server_endpoint_t*, struct connection_t**, struct error_t **);

enum net_op_result close_client_endpoint(const struct client_enpoint_t *, struct error_t **);

enum net_op_result send_data(struct connection_t*, void *, size_t, struct error_t **);

#endif //NET_H