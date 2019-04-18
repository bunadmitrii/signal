#ifndef NET_H
#define NET_H

#include <unistd.h>
#include <stdint.h>

#include "util.h"

struct connection_config_t;

struct connection_config_t *net_allocate_config();
void net_release_config(struct connection_config_t *);
/**
 * Platform-specific format of the local address. 
 * 
 * On unix-like systems it's supposed to be a valid filesystem
 * path of a domain socket to be created.
 */
void net_set_local_communication(struct connection_config_t* config_ptr, const char *local_address);

void net_set_tcp_communication(struct connection_config_t* config_ptr, const char *hostname, uint16_t port, int backlog);

struct client_t;

//TODO: Closing listening socket does not close client's socket accepted by this one
struct server_t{
    void (*shutdown)(struct server_t *this, struct error_t **);
    struct client_t* (*await_client)(struct server_t *this, struct error_t **);
};

void net_send_data(const struct client_t *, void *buf, size_t size, struct error_t **);

void net_close_client(struct client_t *, struct error_t **);

struct server_t* net_initialize_server_endpoint(const struct connection_config_t *config_ptr, struct error_t **thrown);

#endif //NET_H