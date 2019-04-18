#include <sys/socket.h>
#include <sys/un.h>

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <errno.h>

#include "local_endpoint.h"
#include "util.h"
#include "neterr.h"
#include "net.h"

int open_local_server_socket_(const char *local_address, struct error_t** thrown){
    const size_t sun_path_size = sizeof(((struct sockaddr_un*) NULL) -> sun_path);
    const char *const hostname = local_address;
    const size_t hostname_len = strlen(hostname);

    if(sun_path_size < hostname_len + 1){
        ERROR_SET(thrown, server_initialization_error, "Hostname %s is too long. Maximum allowed size is %lu", hostname, sun_path_size - 1);
        return -1;
    }

    printf("Creating socket for local communication... ");
    fflush(stdout);
    int sock_fd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if(sock_fd == -1){
        ERROR_SET(thrown, server_initialization_error, "Unable to create socker. Error code = %d, deatils: %s", errno, strerror(errno));
        // close(sock_fd); Do we really need to close?
        return -1;
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
        ERROR_SET(thrown, server_initialization_error, "Unable to bind socket %d to address %s. Error code = %d, details = %s", sock_fd, hostname, errno, strerror(errno));
        return -1;
    }
    printf("OK.\n");

    printf("Preparing to accept incoming connection on socket %d... ", sock_fd);
    fflush(stdout);
    int listen_result = listen(sock_fd, 0);
    if(listen_result == -1){
        ERROR_SET(thrown, server_initialization_error, "Unable to mark socket %d as a passive socket. Error code = %d, details = %s", sock_fd, errno, strerror(errno));
        return -1;
    }
    printf("OK.\n");

    return sock_fd;
}