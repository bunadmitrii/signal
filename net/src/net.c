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

static enum net_op_result _await_local_connection(connection ** conn_ptr, struct connection_config_t *config_ptr);

enum net_op_result await_connection(struct connection_t** connection_ptr, struct connection_config_t *config_ptr){
    switch(config_ptr -> type){
        case local: {
            return _await_local_connection(connection_ptr, config_ptr);
        }
        default:
            fprintf(stderr, "Unknown type: %d\n", config_ptr -> type);
            return connection_establishment_error;
    }
}


enum net_op_result close_connection(connection *conn_ptr){
    printf("Closing peer file descriptor... ");
    fflush(stdout);
    if(close(conn_ptr -> peer_fd) == -1){
        fprintf(stderr, "\nCannot close peer file descriptor %d. Error code = %d, deatils = %s\n", conn_ptr -> peer_fd, errno, strerror(errno));
        return connection_closing_error;
    }
    printf("OK.\n");

    printf("Closing listening host socket... ");
    fflush(stdout);
    if(close(conn_ptr -> sock_fd) == -1){
        fprintf(stderr, "\nCannot close host socket file descriptor %d. Error code = %d, details = %s\n", conn_ptr -> sock_fd, errno, strerror(errno));
        return connection_closing_error;
    }
    printf("OK.\n");
    return success;
}

enum net_op_result send_data(connection *conn_ptr, void *buf, size_t to_send){
    char *data = (char *) buf;
    const int peer_fd = conn_ptr -> peer_fd;
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

static enum net_op_result _await_local_connection(struct connection_t ** conn_ptr, struct connection_config_t *config_ptr){
    const char *sockname = config_ptr -> host;
    const size_t sockname_len = strlen(sockname);
    const size_t sun_path_len = sizeof(((struct sockaddr_un *) NULL) -> sun_path);
    if(sockname_len > sun_path_len - 1){
        fprintf(stderr, "The hostname exceeds maximum %lu character. Hostname = %s\n", sun_path_len, sockname);
        return connection_establishment_error;
    }
    struct sockaddr_un server_address; 
    memset(&server_address, '\0', sizeof(server_address));
    server_address.sun_family = AF_LOCAL;
    strncpy(server_address.sun_path, sockname, sockname_len);
    
    //TODO: Replace with strncpy
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
        return connection_establishment_error;
    }
    printf("OK.\n");

    printf("Binding socket to address %s... ", server_address.sun_path);
    fflush(stdout);
    int bind_result = bind(sock_fd, (struct sockaddr*) &server_address, server_addrlen);
    if(bind_result == -1){
        fprintf(stderr, "\nCannot bind socket to local address %s. Error code = %d. Details: %s\n", server_address.sun_path, errno, strerror(errno));
        return connection_establishment_error;
    }
    printf("OK.\n");

    printf("Marking socket %d as a passive socket... ", sock_fd);
    fflush(stdout);
    int listen_result = listen(sock_fd, 0);
    if(listen_result == -1){
        fprintf(stderr, "\nCannot mark socket with descriptor %d as a passive socket. Error code = %d. Details: %s\n", sock_fd, errno, strerror(errno));
        return connection_establishment_error;
    }
    printf("OK.\n");

    printf("Waiting for a connection... ");
    fflush(stdout);
    int peer_fd = accept(sock_fd, (struct sockaddr *)&peer_address, &peer_addrlen);
    if(peer_fd == -1){
        fprintf(stderr, "\nError when waiting for a connection. Error code = %d. Details: %s\n", errno, strerror(errno));
        return connection_establishment_error;
    }
    printf("Connected\n");

    struct connection_t tmp = {.sock_fd = sock_fd, .peer_fd = peer_fd};
    struct connection_t *connection_ptr = malloc(sizeof(struct connection_t));
    memcpy(connection_ptr, &tmp, sizeof(tmp));
    *conn_ptr = connection_ptr;
    return success;
}