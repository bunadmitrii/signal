#ifndef NET_H
#define NET_H

#include <unistd.h>

#include "connconfig.h"

enum net_op_result {
    success,
    connection_establishment_error = -1,
    data_transfer_error = -2,
    connection_closing_error = -3
};

typedef struct connection_t connection;

enum net_op_result await_connection(connection **, struct connection_config_t *config_ptr);

enum net_op_result close_connection(connection *);

enum net_op_result send_data(connection *, void *, size_t);

#endif //NET_H