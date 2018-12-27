#ifndef NET_CONFIG_INTERNAL_H
#define NET_CONFIG_INTERNAL_H

#include <stdint.h>

enum connection_type{
    local,
    tcp
};

struct local_conf_t{
    const char *const local_address;
};

struct tcp_conf_t{
    const char *const hostname;
    const uint32_t port;
};

union conf_t{
    const struct local_conf_t local_conf;
    const struct tcp_conf_t tcp_conf;
};

struct connection_config_t{
    const enum connection_type type;
    const union conf_t conf;
};

struct server_endpoint_t{
    const enum connection_type type;
    const int sock_fd;
    const union conf_t conf;
};

struct client_enpoint_t{
    const int peer_fd;
};

#endif //NET_CONFIG_INTERNAL_H