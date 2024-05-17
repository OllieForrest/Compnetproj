#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "server.h"

#define PROBE_MSG "PROBE:"
#define PROBE_RESPONSE "PROBE_RESPONSE:"

void handle_udp(int udp_sock, const char *station_name, Neighbor *neighbors, int neighbor_count, pthread_mutex_t *mutex) {
    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);
    char buffer[BUF_SIZE];
    ssize_t msg_len;

    while ((msg_len = recvfrom(udp_sock, buffer, BUF_SIZE, 0, (struct sockaddr *)&cliaddr, &len)) > 0) {
        buffer[msg_len] = '\0';

        if (strncmp(buffer, PROBE_MSG, strlen(PROBE_MSG)) == 0) {
            // Respond to the probe with a probe response
            char response[BUF_SIZE];
            snprintf(response, sizeof(response), "%s%s", PROBE_RESPONSE, station_name);
            sendto(udp_sock, response, strlen(response), 0, (struct sockaddr *)&cliaddr, len);
            printf("Received probe from %s:%d, sent probe response: %s\n",
                   inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), response); // Debugging output
        } else if (strncmp(buffer, PROBE_RESPONSE, strlen(PROBE_RESPONSE)) == 0) {
            // This is a probe response
            const char *response_station_name = buffer + strlen(PROBE_RESPONSE);

            // Find the neighbor that matches the sender's port
            for (int i = 0; i < neighbor_count; i++) {
                if (cliaddr.sin_port == htons(neighbors[i].udp_port)) {
                    pthread_mutex_lock(mutex);
                    strncpy(neighbors[i].station_name, response_station_name, sizeof(neighbors[i].station_name) - 1);
                    neighbors[i].station_name[sizeof(neighbors[i].station_name) - 1] = '\0';
                    neighbors[i].response_received = 1; // Set the response received flag
                    pthread_mutex_unlock(mutex);
                    printf("Received probe response from port %d - Station Name: %s\n",
                           ntohs(cliaddr.sin_port), neighbors[i].station_name); // Debugging output
                    break;
                }
            }
        } else {
            const char *response = "UDP connection active.";
            sendto(udp_sock, response, strlen(response), 0, (struct sockaddr *)&cliaddr, len);
        }
    }
}

void *udp_thread_func(void *arg) {
    UdpThreadArgs *args = (UdpThreadArgs *)arg;
    handle_udp(args->socket, args->station_name, args->neighbors, args->neighbor_count, args->mutex);
    return NULL;
}
