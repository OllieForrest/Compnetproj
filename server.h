#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define BUF_SIZE 1024
#define INITIAL_SCHEDULE_CAPACITY 10

typedef struct {
    char departure_time[6];
    char departure_stop[50];
    char destination_station[50];
    char arrival_time[6];
} BusSchedule;

typedef struct {
    char ip[INET_ADDRSTRLEN];
    int udp_port;
    char station_name[50];
    int response_received; // Add this field
} Neighbor;

typedef struct {
    int socket;
    const char *station_name;
    Neighbor *neighbors;
    int neighbor_count;
    BusSchedule *schedule_array;
    int schedule_count;
    pthread_mutex_t *mutex;
} ThreadArgs;

typedef struct {
    int socket;
    const char *station_name;
    Neighbor *neighbors;
    int neighbor_count;
    pthread_mutex_t *mutex;
} UdpThreadArgs;

void handle_tcp(int tcp_sock, const char *station_name, Neighbor *neighbors, int neighbor_count, BusSchedule *schedule_array, int schedule_count, pthread_mutex_t *mutex);
void *tcp_thread_func(void *arg);
void handle_udp(int udp_sock, const char *station_name, Neighbor *neighbors, int neighbor_count, pthread_mutex_t *mutex);
void *udp_thread_func(void *arg);
void read_timetable(const char *station_name); 
void probe_neighbors(Neighbor *neighbors, int neighbor_count, const char *station_name, pthread_mutex_t *mutex);
#endif // SERVER_H
