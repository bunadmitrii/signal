#include <errno.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include "tcp_endpoint.h"

struct server_endpoint_t* create_tcp_endpoint_(struct tcp_conf_t tcp_conf, struct error_t** thrown){
    printf("Opening TCP socket... ");
    fflush(stdout);
    int tcp_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(tcp_socket_fd == -1){
        ERROR_SET(thrown, errno, "Unable to create tcp socket. Error code = %d, details = %s", errno, strerror(errno));
        return NULL;
    }
    printf("OK!\n");

    struct in_addr inp;
    printf("Translating %s to a binary network byte order format... ", tcp_conf.hostname);
    fflush(stdout);
    if(inet_aton(tcp_conf.hostname, &inp) == 0){
        ERROR_SET(thrown, errno, "Unable to conver address %s to a binary format", tcp_conf.hostname);
        return NULL;
    }
    printf("OK!\n");
    struct sockaddr_in sockaddr_in = {.sin_family = AF_INET, .sin_port = htons(tcp_conf.port), .sin_addr = {inp.s_addr}};
    
    printf("Binding socket to the configured address... ");
    fflush(stdout);
    if(bind(tcp_socket_fd, (struct sockaddr *) &sockaddr_in, sizeof(sockaddr_in)) == -1){
        ERROR_SET(thrown, errno, "Unable to bind socket with file descriptor %d with port %d, host %s. Error code = %d, details = %s", tcp_socket_fd, tcp_conf.port, tcp_conf.hostname, errno, strerror(errno));
        return NULL;
    }
    printf("OK!\n");

    printf("Marking socket reffered to by file descriptor %d as a passive socket... ", tcp_socket_fd);
    fflush(stdout);
    if(listen(tcp_socket_fd, tcp_conf.backlog) == -1){
        ERROR_SET(thrown, errno, "Unable to mark %d as a passive socket. Error code = %d, details = %s", tcp_socket_fd, errno, strerror(errno));
        return NULL;
    }
    printf("OK!\n");
    
    struct server_endpoint_t tmp = {.type = tcp, .sock_fd = tcp_socket_fd, .conf = {.tcp_conf = tcp_conf}};
    struct server_endpoint_t *server_endpoint_ptr = malloc(sizeof(struct server_endpoint_t));
    if(server_endpoint_ptr != NULL){
        memcpy(server_endpoint_ptr, &tmp, sizeof(tmp));
    }
    return server_endpoint_ptr;
}

void close_tcp_endpoint_(struct server_endpoint_t server_endpoint, struct error_t **thrown){
    if(close(server_endpoint.sock_fd) == -1){
        ERROR_SET(thrown, errno, "Cannot close tcp server endpoint reffered to by descriptor %d. Error code = %d, details = %s", server_endpoint.sock_fd, errno, strerror(errno));
    }
}