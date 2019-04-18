#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <sys/un.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#include "net.h"
#include "neterr.h"
#include "util.h"

#include "internal/tcp_endpoint.h"
#include "internal/local_endpoint.h"

enum connection_type{
    local,
    tcp
};

struct connection_config_t{
    enum connection_type type;
    union {
        struct {
            const char *local_address;
        } local_conf;
        struct{
            const char *hostname;
            uint16_t port;
            int backlog;
        } tcp_conf;
    } conf;
};

struct client_t{
    int client_fd;
    socklen_t addrlen;
    enum connection_type type;
    union {
        struct sockaddr_un sockaddr_un;
        struct sockaddr_in sockaddr_in;
    } client_addr;
};

struct server_impl_t{
    struct server_t base;
    int server_socket_fd;
    enum connection_type type;
    const char *local_address;
};

static struct client_t* net_await_client(struct server_t *srv_endpoint_ptr, struct error_t **thrown){
    const struct server_impl_t *srv_ptr = (const struct server_impl_t*) srv_endpoint_ptr;
    const int server_sock_fd = srv_ptr -> server_socket_fd;
    struct sockaddr *peer_address = NULL;
    socklen_t peer_addrlen = 0;
    struct client_t *client_ptr = malloc(sizeof(*client_ptr));
    client_ptr -> type = srv_ptr -> type;
    if(srv_ptr -> type == local){
        peer_address = malloc(sizeof(struct sockaddr_un));
        peer_addrlen = sizeof(struct sockaddr_un);
    } else if(srv_ptr -> type == tcp){
        peer_address = malloc(sizeof(struct sockaddr_in));
        peer_addrlen = sizeof(struct sockaddr_in);
    }
    printf("Waiting for connection... ");
    fflush(stdout);
    int peer_fd = accept(server_sock_fd, (struct sockaddr *) &peer_address, &peer_addrlen);
    if(peer_fd == -1){
        ERROR_SET(thrown, connection_failure, "Peer connection error on server socker fd = %d. Error code = %d, details = %s", server_sock_fd, errno, strerror(errno));
        return NULL;
    }
    printf("OK.\n");
    
    memcpy(&(client_ptr -> client_addr), peer_address, peer_addrlen);

    client_ptr -> addrlen = peer_addrlen;
    client_ptr -> client_fd = peer_fd;
    return client_ptr;
}

static void net_shutdown_server(struct server_t *server_ptr, struct error_t ** thrown){
    printf("Shutting down server...");
    fflush(stdout);
    struct server_impl_t *server_impl_ptr = (struct server_impl_t*) server_ptr; 
    const int server_socket_fd = server_impl_ptr -> server_socket_fd;
    const enum connection_type type = server_impl_ptr -> type;
    if(type == local){
        const char *local_address = server_impl_ptr -> local_address;
        if (unlink(local_address) != -1){
            int error_code = errno;
                ERROR_SET(thrown,
                error_code,
                "Cannot unlink the domain socket reffered to by path %s. Error code = %d, details: %s",
                local_address,
                error_code,
                strerror(error_code)
            );
        }
    }
    if(close(server_socket_fd) == -1){
        int error_code = errno;
        ERROR_SET(
            thrown,
            errno,
            "Cannot shutdown server refferred to by file descriptor %d. Error code = %d, details: %s",
            server_socket_fd,
            error_code,
            strerror(error_code)
        );
    }
    printf("OK\n");
}

void net_close_client(struct client_t *client_ptr, struct error_t **thrown){
    int close_result = close(client_ptr -> client_fd);
    if(close_result == -1)
        ERROR_SET(
            thrown,
            closing_client_error,
            "Cannot close client reffering to by file descriptor %d. Details %s",
            client_ptr -> client_fd,
            strerror(errno)
        );
}

struct server_t* net_initialize_server_endpoint(const struct connection_config_t *config_ptr, struct error_t **thrown){
    struct server_impl_t *server_ptr = malloc(sizeof(*server_ptr));
    server_ptr -> type = config_ptr -> type;
    //TODO: Check if socket fd is valid and return NULL if not
    switch(config_ptr -> type){
        case local: {
            const char *local_address = config_ptr -> conf.local_conf.local_address;
            server_ptr -> server_socket_fd = open_local_server_socket_(
                local_address,
                thrown);
            server_ptr -> local_address = local_address;
            break;
        }
        case tcp:
            server_ptr -> server_socket_fd = open_tcp_server_socket_(
                config_ptr -> conf.tcp_conf.hostname,
                config_ptr -> conf.tcp_conf.port,
                config_ptr -> conf.tcp_conf.backlog,
                thrown
            );
            break;
        default: {
            ERROR_SET(thrown, server_initialization_error, "Unknown type %d", config_ptr -> type);
            return NULL;
        }
    }
    server_ptr -> base.await_client = &net_await_client;
    server_ptr -> base.shutdown = &net_shutdown_server;
    return (struct server_t *)server_ptr;
}


void net_close_client_endpoint(const struct client_t* client_endpoint_ptr, struct error_t **thrown){
    printf("Closing peer file descriptor... ");
    enum connection_type type = client_endpoint_ptr -> type;
    if(type == local){
        struct sockaddr_un domain_socket_client = client_endpoint_ptr -> client_addr.sockaddr_un;
        printf("Local client address = %s\n", domain_socket_client.sun_path);
    } else if (type == tcp) {
        struct sockaddr_in tcp_client = client_endpoint_ptr -> client_addr.sockaddr_in;
        printf("TCP client address = %s\n", inet_ntoa(tcp_client.sin_addr));
    }
    fflush(stdout);
    int peer_fd = client_endpoint_ptr -> client_fd;
    if(close(peer_fd) == -1){
        ERROR_SET(thrown, closing_client_error, "Cannot close peer file descriptor %d. Error code = %d, deatils = %s", peer_fd, errno, strerror(errno));
        return;
    }
    printf("OK.\n");
}

void net_send_data(const struct client_t *client_ptr, void *buf, size_t to_send, struct error_t **thrown){
    char *data = (char *) buf;
    const int peer_fd = client_ptr -> client_fd;
    ssize_t written = write(peer_fd, data, to_send);
    while (written > 0){
        written = write(peer_fd, data + written, to_send - written);
    }
    if(written < 0){
        ERROR_SET(thrown, data_transfer_error, "Cannot write data to peer socket %d. Error code = %d, details = %s", peer_fd, errno, strerror(errno));
        return;
    }
}

struct connection_config_t *net_allocate_config(){
    return malloc(sizeof(struct connection_config_t));
}

void net_set_local_communication(struct connection_config_t* config_ptr, const char *local_address){
    //TODO: Doesn't it cause UB if I assign to a member of a member of a union,
    //TODO: not to a member of union itself
    config_ptr -> conf.local_conf.local_address = local_address;
}

void net_set_tcp_communication(struct connection_config_t* config_ptr, const char *hostname, uint16_t port, int backlog){
    config_ptr -> conf.tcp_conf.hostname = hostname;
    config_ptr -> conf.tcp_conf.port = port;
    config_ptr -> conf.tcp_conf.backlog = backlog;
}

void net_release_config(struct connection_config_t * config_ptr){
    free(config_ptr);
}