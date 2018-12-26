#ifndef NET_H
#define NET_H

#include <unistd.h>

#include "connconfig.h"

enum net_op_result {
    success,
    server_initialization_error = -1,
    connection_establishment_error = -2,
    data_transfer_error = -3,
    connection_closing_error = -4
};

typedef struct server_endpoint_t server_endpoint;

typedef struct client_enpoint_t client_endpoint;

typedef struct connection_t {
    const struct server_endpoint_t *srv_endpoint_ptr;
    const struct client_enpoint_t *client_endpoint_ptr;
} connection;

enum net_op_result initialize_server_endpoint(server_endpoint **, struct connection_config_t *config_ptr);

enum net_op_result release_server_endpoing(const struct server_endpoint_t *);

enum net_op_result await_connection(const server_endpoint*, connection**);

enum net_op_result close_client_endpoint(const struct client_enpoint_t *);

enum net_op_result send_data(connection *, void *, size_t);

#endif //NET_H