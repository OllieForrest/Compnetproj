#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "server.h"

int create_and_bind_socket(int port, int type) {
    int sockfd;
    struct sockaddr_in addr;
    int opt = 1;

    if ((sockfd = socket(AF_INET, type, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <station_name> <webPort> <udpPort> [localhost:port]...\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *station_name = argv[1];
    int webPort = atoi(argv[2]);
    int udpPort = atoi(argv[3]);
    int neighbor_count = argc - 4;

    Neighbor *neighbors = (Neighbor *)malloc(neighbor_count * sizeof(Neighbor));
    if (neighbors == NULL) {
        perror("Memory allocation failed");
        return 1;
    }

    for (int i = 0; i < neighbor_count; i++) {
        char *token = strtok(argv[4 + i], ":");
        strcpy(neighbors[i].ip, token);
        token = strtok(NULL, ":");
        neighbors[i].udp_port = atoi(token);
        memset(neighbors[i].station_name, 0, sizeof(neighbors[i].station_name));
    }

    printf("Station Name: %s\n", station_name);
    printf("webPort: %d\n", webPort);
    printf("udpPort: %d\n", udpPort);
    printf("Neighbouring Addresses:\n");
    for (int i = 0; i < neighbor_count; i++) {
        printf("Neighbour %d: %s:%d\n", i + 1, neighbors[i].ip, neighbors[i].udp_port);
    }

    // Read the timetable for the station
    read_timetable(station_name);

    int tcp_sock, udp_sock;

    // TCP socket for HTTP requests
    tcp_sock = create_and_bind_socket(webPort, SOCK_STREAM);
    if (listen(tcp_sock, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // UDP socket for inter-station communication
    udp_sock = create_and_bind_socket(udpPort, SOCK_DGRAM);

    printf("Server is running on TCP port %d and UDP port %d\n", webPort, udpPort);

    ThreadArgs tcp_args = {tcp_sock, "", neighbors, neighbor_count};
    ThreadArgs udp_args = {udp_sock, "", neighbors, neighbor_count};
    strncpy(tcp_args.station_name, station_name, sizeof(tcp_args.station_name) - 1);
    strncpy(udp_args.station_name, station_name, sizeof(udp_args.station_name) - 1);

    pthread_t tcp_thread, udp_thread;
    pthread_create(&tcp_thread, NULL, tcp_thread_func, &tcp_args);
    pthread_create(&udp_thread, NULL, udp_thread_func, &udp_args);

    pthread_join(tcp_thread, NULL);
    pthread_join(udp_thread, NULL);

    free(neighbors);
    free(schedule_array);

    return 0;
}
