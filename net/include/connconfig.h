#ifndef CONNCONFIG_H
#define CONNCONFIG_H
#include <stdint.h>

/**
 * Platform-specific format of the local address. 
 * 
 * On unix-like systems it's supposed to be a valid filesystem
 * path of a domain socket to be created.
 */
struct connection_config_t *allocate_local(const char *local_address);

struct connection_config_t *allocate_tcp(const char *hostname, uint16_t port, int backlog);

void release_config(struct connection_config_t *);

#endif //CONNCONFIG_H