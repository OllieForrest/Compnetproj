#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "server.h"

void handle_tcp(int tcp_sock, const char *station_name) {
    char buffer[BUF_SIZE];
    int new_socket;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    while ((new_socket = accept(tcp_sock, (struct sockaddr *)&client_addr, &addr_len)) >= 0) {
        memset(buffer, 0, BUF_SIZE);
        read(new_socket, buffer, BUF_SIZE);

        // Respond with a simple message
        const char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 21\r\n\r\nTCP connection active.";
        send(new_socket, response, strlen(response), 0);

        close(new_socket);
    }
}

void *tcp_thread_func(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    handle_tcp(args->socket, args->station_name);
    return NULL;
}
