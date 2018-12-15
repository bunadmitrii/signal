#ifndef NET_H
#define NET_H

#include "connconfig.h"

enum connect_status {
    connected,
    error = -1
};

typedef struct connection_t connection;

enum connect_status await_connection(connection **, struct connection_config_t *config_ptr);

#endif //NET_H