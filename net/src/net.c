//for string.h::strdup to be available
#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <sys/un.h>
#include <sys/socket.h>

#include "net.h"

#include "internal/config_internal.h"

struct server_endpoint_t{
    const enum connection_type type;
    const int sock_fd;
    const union conf_t conf;
};

struct client_enpoint_t{
    const int peer_fd;
};

static struct server_endpoint_t* _craete_local_endpoint(struct local_conf_t local_conf_ptr);
static enum net_op_result _close_local_endpoint(const struct local_conf_t, int);

enum net_op_result initialize_server_endpoint(struct server_endpoint_t **srv_endpoint_ptr, struct connection_config_t *config_ptr){
    switch(config_ptr -> type){
        case local: {
            struct server_endpoint_t *tmp = _craete_local_endpoint(config_ptr -> conf.local_conf);
            if(tmp != NULL){
                *srv_endpoint_ptr = tmp;
                return success;
            } else 
                return server_initialization_error;
        }
        default:
            fprintf(stderr, "Unknown type: %d\n", config_ptr -> type);
            return connection_establishment_error;
    }
}

enum net_op_result release_server_endpoing(const struct server_endpoint_t *srv_endpoint_ptr){
    enum connection_type type = srv_endpoint_ptr -> type;
    switch(type){
        case local: {
            const int sock_fd = srv_endpoint_ptr -> sock_fd;
            return _close_local_endpoint(srv_endpoint_ptr -> conf.local_conf, sock_fd);
        }
        default: 
            fprintf(stderr, "Cannot close connection with unknown type %d\n", type);
            return connection_closing_error;
    }
}

enum net_op_result await_connection(const struct server_endpoint_t *srv_endpoint_ptr, connection** conn_ptr){
    const int server_sock_fd = srv_endpoint_ptr -> sock_fd;
    struct sockaddr_un peer_address;
    memset(&peer_address, '\0', sizeof(peer_address));
    socklen_t peer_addrlen = 0;
    printf("Waiting for local connection... ");
    fflush(stdout);
    int peer_fd = accept(server_sock_fd, (struct sockaddr *) &peer_address, &peer_addrlen);
    if(peer_fd == -1){
        fprintf(stderr, "\nPeer connection error on server socker fd = %d. Error code = %d, details = %s\n", server_sock_fd, errno, strerror(errno));
        return connection_establishment_error;
    }
    printf("OK.\n");
    struct client_enpoint_t *client_endpoint_ptr = malloc(sizeof(*client_endpoint_ptr));
    memcpy(client_endpoint_ptr, &peer_fd, sizeof(*client_endpoint_ptr));
    struct connection_t tmp = {.srv_endpoint_ptr = srv_endpoint_ptr, .client_endpoint_ptr = client_endpoint_ptr};
    struct connection_t *ptr = malloc(sizeof(**conn_ptr));
    memcpy(ptr, &tmp, sizeof(tmp));
    *conn_ptr = ptr;
    return success;
}

enum net_op_result close_client_endpoint(const struct client_enpoint_t* client_endpoint_ptr){
    printf("Closing peer file descriptor... ");
    fflush(stdout);
    int peer_fd = client_endpoint_ptr -> peer_fd;
    if(close(peer_fd) == -1){
        fprintf(stderr, "\nCannot close peer file descriptor %d. Error code = %d, deatils = %s\n", peer_fd, errno, strerror(errno));
        return connection_closing_error;
    }
    printf("OK.\n");
    return success;
}

enum net_op_result send_data(connection *conn_ptr, void *buf, size_t to_send){
    char *data = (char *) buf;
    const int peer_fd = conn_ptr -> client_endpoint_ptr -> peer_fd;
    ssize_t written = write(peer_fd, data, to_send);
    while (written > 0){
        written = write(peer_fd, data + written, to_send - written);
    }
    if(written < 0){
        fprintf(stderr, "Cannot write data to peer socket %d. Error code = %d, details = %s\n", peer_fd, errno, strerror(errno));
        return data_transfer_error;
    }
    return success;
}

//TODO: Exteranal linkage. Is it UB (previous one declared is static)
struct server_endpoint_t* _craete_local_endpoint(struct local_conf_t local_conf){
    const size_t sun_path_size = sizeof(((struct sockaddr_un*) NULL) -> sun_path);
    const char *const hostname = local_conf.local_address;
    const size_t hostname_len = strlen(hostname);

    if(sun_path_size < hostname_len + 1){
        fprintf(stderr, "Hostname %s is too long. Maximum allowed size is %lu", hostname, sun_path_size - 1);
        return NULL;
    }

    printf("Creating socket for local communication... ");
    fflush(stdout);
    int sock_fd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if(sock_fd == -1){
        close(sock_fd);
        fprintf(stderr, "\nUnable to create socker. Error code = %d, deatils: %s", errno, strerror(errno));
        return NULL;
    }
    printf("OK.\n");

    struct sockaddr_un server_address;
    memset(&server_address, '\0', sizeof(struct sockaddr_un));
    server_address.sun_family = AF_LOCAL;
    strncpy(server_address.sun_path, hostname, hostname_len + 1);
    const socklen_t addrlen = offsetof(struct sockaddr_un, sun_path) + hostname_len + 1;

    printf("Trying to bind socket %d to address %s... ", sock_fd, hostname);
    fflush(stdout);
    int bind_result = bind(sock_fd, (struct sockaddr*) &server_address, addrlen);
    if(bind_result == -1){
        fprintf(stderr, "\nUnable to bind socket %d to address %s. Error code = %d, details = %s\n", sock_fd, hostname, errno, strerror(errno));
        return NULL;
    }
    printf("OK.\n");

    printf("Preparing to accept incoming connection on socket %d... ", sock_fd);
    fflush(stdout);
    int listen_result = listen(sock_fd, 0);
    if(listen_result == -1){
        fprintf(stderr, "\nUnable to mark socket %d as a passive socket. Error code = %d, details = %s\n", sock_fd, errno, strerror(errno));
        return NULL;
    }
    printf("OK.\n");

    struct server_endpoint_t tmp = {
        .type = local,
        .sock_fd = sock_fd,
        .conf = {local_conf}
    };
    struct server_endpoint_t *srv_endpoint = calloc(1, sizeof(struct server_endpoint_t));
    memcpy(srv_endpoint, &tmp, sizeof(tmp));

    return srv_endpoint;
}

static enum net_op_result _close_local_endpoint(const struct local_conf_t local_conf_ptr, int sock_fd){
    const char *const local_address = local_conf_ptr.local_address;
    printf("Closing local endpoint... ");
    fflush(stdout);
    if(close(sock_fd) == -1){
        fprintf(stderr, "\nCannot close local socket file descriptor %d. Error code = %d, details = %s\n", sock_fd, errno, strerror(errno));
        return connection_closing_error;
    }
    printf("OK.\n");

    printf("Unlinkind local socket... ");
    fflush(stdout);
    if(unlink(local_address) == -1){
        fprintf(stderr, "\nCannot unlink local socket %s. Error code = %d, details = %s\n", local_address, errno, strerror(errno));
        return connection_closing_error;
    }
    printf("OK\n");

    return success;
}
