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

typedef struct connection_t connection;

typedef struct server_endpoint_t server_endpoint;

enum net_op_result initialize_server_endpoint(server_endpoint **, struct connection_config_t *config_ptr);

enum net_op_result await_connection(const server_endpoint*, connection**);

enum net_op_result close_connection(connection *);

enum net_op_result send_data(connection *, void *, size_t);

#endif //NET_H