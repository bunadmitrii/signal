#include <errno.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include "net.h"
#include "tcp_endpoint.h"

int open_tcp_server_socket_(const char *hostname, uint16_t port, int backlog, struct error_t** thrown){
    printf("Opening TCP socket... ");
    fflush(stdout);
    int tcp_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(tcp_socket_fd == -1){
        ERROR_SET(thrown, errno, "Unable to create tcp socket. Error code = %d, details = %s", errno, strerror(errno));
        return -1;
    }
    printf("OK!\n");

    struct in_addr inp;
    printf("Translating %s to a binary network byte order format... ", hostname);
    fflush(stdout);
    if(inet_aton(hostname, &inp) == 0){
        ERROR_SET(thrown, errno, "Unable to conver address %s to a binary format", hostname);
        return -1;
    }
    printf("OK!\n");
    struct sockaddr_in sockaddr_in = {.sin_family = AF_INET, .sin_port = htons(port), .sin_addr = {inp.s_addr}};
    
    printf("Binding socket to the configured address... ");
    fflush(stdout);
    if(bind(tcp_socket_fd, (struct sockaddr *) &sockaddr_in, sizeof(sockaddr_in)) == -1){
        ERROR_SET(thrown, errno, "Unable to bind socket with file descriptor %d with port %d, host %s. Error code = %d, details = %s", tcp_socket_fd, port, hostname, errno, strerror(errno));
        return -1;
    }
    printf("OK!\n");

    printf("Marking socket reffered to by file descriptor %d as a passive socket... ", tcp_socket_fd);
    fflush(stdout);
    if(listen(tcp_socket_fd, backlog) == -1){
        ERROR_SET(thrown, errno, "Unable to mark %d as a passive socket. Error code = %d, details = %s", tcp_socket_fd, errno, strerror(errno));
        return -1;
    }
    printf("OK!\n");
    
    return tcp_socket_fd;
}