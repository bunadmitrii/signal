#ifndef NET_TYPES_H
#define NET_TYPES_H

struct server_endpoint_t;

struct client_enpoint_t;

struct connection_t {
    const struct server_endpoint_t *srv_endpoint_ptr;
    const struct client_enpoint_t *client_endpoint_ptr;
};

#endif //NET_TYPES_H