#ifndef TCP_ENDPOINT_INTERNAL_H
#define TCP_ENDPOINT_INTERNAL_H

#include <stdint.h>

#include "util.h"
#include "net.h"

int open_tcp_server_socket_(const char *hostname, uint16_t port, int backlog, struct error_t** thrown);

#endif //TCP_ENDPOINT_INTERNAL_H