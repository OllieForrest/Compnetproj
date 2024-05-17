#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "server.h"

void handle_udp(int udp_sock, const char *station_name) {
    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);
    char buffer[BUF_SIZE];
    ssize_t msg_len;

    while ((msg_len = recvfrom(udp_sock, buffer, BUF_SIZE, 0, (struct sockaddr *)&cliaddr, &len)) > 0) {
        buffer[msg_len] = '\0';

        const char *response = "UDP connection active.";
        sendto(udp_sock, response, strlen(response), 0, (struct sockaddr *)&cliaddr, len);
    }
}

void *udp_thread_func(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    handle_udp(args->socket, args->station_name);
    return NULL;
}
