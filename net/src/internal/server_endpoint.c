#include <sys/socket.h>
#include <sys/un.h>

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <errno.h>

#include "server_endpoint.h"

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

enum net_op_result _close_local_endpoint(const struct local_conf_t local_conf_ptr, int sock_fd){
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