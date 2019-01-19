#ifndef NET_H
#define NET_H

#include <unistd.h>

#include "connconfig.h"
#include "connection.h"
#include "util.h"

enum net_error_codes {
    server_initialization_error = 1,
    connection_failure,
    closing_client_error,
    data_transfer_error,
    closing_server_error
};

struct server_endpoint_t* initialize_server_endpoint(struct connection_config_t *, struct error_t **);

void release_server_endpoing(const struct server_endpoint_t*, struct error_t **);

struct connection_t* await_connection(const struct server_endpoint_t*, struct error_t **);

void close_client_endpoint(const struct client_enpoint_t *, struct error_t **);

//TODO: Should we return the amount actually sent?
void send_data(struct connection_t*, void *, size_t, struct error_t **);

#endif //NET_H