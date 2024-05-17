#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "server.h"

extern BusSchedule *schedule_array;
extern int schedule_count;
extern int schedule_capacity;

#define BUF_SIZE 1024
#define PROBE_MSG "PROBE:"

void send_probe(const char *ip, int port, const char *station_name) {
    int sockfd;
    struct sockaddr_in servaddr;
    char probe_msg[BUF_SIZE];
    char target_ip[INET_ADDRSTRLEN];

    // Convert "localhost" to "127.0.0.1"
    if (strcmp(ip, "localhost") == 0) {
        strcpy(target_ip, "127.0.0.1");
    } else {
        strcpy(target_ip, ip);
    }

    snprintf(probe_msg, sizeof(probe_msg), "%s%s", PROBE_MSG, station_name);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        return;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    inet_pton(AF_INET, target_ip, &servaddr.sin_addr);

    sendto(sockfd, probe_msg, strlen(probe_msg), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    close(sockfd);
}

void probe_neighbors(Neighbor *neighbors, int neighbor_count, const char *station_name, pthread_mutex_t *mutex) {
    int all_responded = 0;

    while (!all_responded) {
        all_responded = 1;
        for (int i = 0; i < neighbor_count; i++) {
            pthread_mutex_lock(mutex);
            if (!neighbors[i].response_received) {  // Check if the neighbor's response was not yet received
                send_probe(neighbors[i].ip, neighbors[i].udp_port, station_name);
                printf("Sent probe to %s:%d\n", neighbors[i].ip, neighbors[i].udp_port); // Debugging output
                all_responded = 0;  // At least one neighbor hasn't responded yet
            }
            pthread_mutex_unlock(mutex);
        }
        sleep(2);  // Wait for 2 seconds before sending the next round of probes
    }
    printf("All neighbors have responded.\n");
}

void* probe_thread_func(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    probe_neighbors(args->neighbors, args->neighbor_count, args->station_name, args->mutex);
    return NULL;
}

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
        neighbors[i].response_received = 0; // Initialize the response received flag
    }

    printf("Station Name: %s\n", station_name);
    printf("webPort: %d\n", webPort);
    printf("udpPort: %d\n", udpPort);
    printf("Neighbouring Addresses:\n");
    for (int i = 0; i < neighbor_count; i++) {
        printf("Neighbour %d: %s:%d\n", i + 1, neighbors[i].ip, neighbors[i].udp_port);
    }

    read_timetable(station_name);

    int tcp_sock = create_and_bind_socket(webPort, SOCK_STREAM);
    if (listen(tcp_sock, 10) < 0) {
        perror("listen");
        close(tcp_sock);
        exit(EXIT_FAILURE);
    }

    int udp_sock = create_and_bind_socket(udpPort, SOCK_DGRAM);

    pthread_t tcp_thread, udp_thread, probe_thread;
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    ThreadArgs tcp_args = {tcp_sock, station_name, neighbors, neighbor_count, schedule_array, schedule_count, &mutex};
    UdpThreadArgs udp_args = {udp_sock, station_name, neighbors, neighbor_count, &mutex};

    pthread_create(&tcp_thread, NULL, tcp_thread_func, &tcp_args);
    pthread_create(&udp_thread, NULL, udp_thread_func, &udp_args);
    pthread_create(&probe_thread, NULL, probe_thread_func, &tcp_args); // Reusing tcp_args for probing

    pthread_join(tcp_thread, NULL);
    pthread_join(udp_thread, NULL);
    pthread_join(probe_thread, NULL);

    pthread_mutex_destroy(&mutex);
    close(tcp_sock);
    close(udp_sock);
    free(neighbors);
    free(schedule_array);

    return 0;
}
