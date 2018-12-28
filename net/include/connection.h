#ifndef NET_TYPES_H
#define NET_TYPES_H

typedef struct server_endpoint_t server_endpoint;

typedef struct client_enpoint_t client_endpoint;

typedef struct connection_t {
    const struct server_endpoint_t *srv_endpoint_ptr;
    const struct client_enpoint_t *client_endpoint_ptr;
} connection;

#endif //NET_TYPES_H