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

struct connection_t{
    const int sock_fd;
    const int peer_fd;
};

static enum connect_status _await_local_connection(connection ** conn_ptr, struct connection_config_t *config_ptr);

enum connect_status await_connection(struct connection_t** connection_ptr, struct connection_config_t *config_ptr){
    switch(config_ptr -> type){
        case local: {
            struct connection_t *connection_ptr;
            return _await_local_connection(&connection_ptr, config_ptr);
        }
        default:
            fprintf(stderr, "Unknown type: %d\n", config_ptr -> type);
            return error;
    }
}

static enum connect_status _await_local_connection(struct connection_t ** conn_ptr, struct connection_config_t *config_ptr){
    const char *sockname = config_ptr -> host;
    size_t sockname_len = strlen(sockname);
    if(sockname_len > 107){
        fprintf(stderr, "The hostname exceeds maximum 107 character. Hostname = %s\n", sockname);
        return error;
    }
    const struct sockaddr_un server_address = {.sun_family = AF_LOCAL,};
    memcpy(&(server_address.sun_path), sockname, sockname_len + 1);
    const socklen_t server_addrlen = offsetof(struct sockaddr_un, sun_path) + strlen(server_address.sun_path) + 1;

    struct sockaddr_un peer_address;
    memset(&peer_address, '\0', sizeof(peer_address));
    socklen_t peer_addrlen = 0;

    //TODO: Too many boilerplate with flushing and printing error messages.
    printf("Creating socket... ");
    fflush(stdout);
    //TODO: Currently protocol is set to 0 under assumption
    //TODO: that the there is only a single protocol exists 
    //TODO: within a given protocol family
    int sock_fd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if(sock_fd == -1){
        fprintf(stderr, "\nCannot create socket for local communication. Error code = %d. Details: %s\n", errno, strerror(errno));
        return error;
    }
    printf("OK.\n");

    printf("Binding socket to address %s... ", server_address.sun_path);
    fflush(stdout);
    int bind_result = bind(sock_fd, (struct sockaddr*) &server_address, server_addrlen);
    if(bind_result == -1){
        fprintf(stderr, "\nCannot bind socket to local address %s. Error code = %d. Details: %s\n", server_address.sun_path, errno, strerror(errno));
        return error;
    }
    printf("OK.\n");

    printf("Marking socket %d as a passive socket... ", sock_fd);
    fflush(stdout);
    int listen_result = listen(sock_fd, 0);
    if(listen_result == -1){
        fprintf(stderr, "\nCannot mark socket with descriptor %d as a passive socket. Error code = %d. Details: %s\n", sock_fd, errno, strerror(errno));
        return error;
    }
    printf("OK.\n");

    printf("Waiting for a connection... ");
    fflush(stdout);
    int peer_fd = accept(sock_fd, (struct sockaddr *)&peer_address, &peer_addrlen);
    if(peer_fd == -1){
        fprintf(stderr, "\nError when waiting for a connection. Error code = %d. Details: %s\n", errno, strerror(errno));
        return error;
    }
    printf("Connected\n");

    struct connection_t tmp = {.sock_fd = sock_fd, .peer_fd = peer_fd};
    struct connection_t *connection_ptr = malloc(sizeof(struct connection_t));
    memcpy(connection_ptr, &tmp, sizeof(tmp));
    *conn_ptr = connection_ptr;
    return connected;
}