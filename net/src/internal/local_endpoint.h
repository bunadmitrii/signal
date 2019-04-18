#ifndef NET_LOCAL_ENDPOINT_INTERNAL_H
#define NET_LOCAL_ENDPOINT_INTERNAL_H

#include "util.h"

int open_local_server_socket_(const char *local_address, struct error_t**);

#endif //NET_LOCAL_ENDPOINT_INTERNAL_H