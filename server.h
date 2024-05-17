#ifndef SERVER_H
#define SERVER_H

#include <pthread.h>

#define BUF_SIZE 1024
#define INITIAL_SCHEDULE_CAPACITY 10
#define BUFFER_SIZE 1024

typedef struct {
    char ip[16];
    int udp_port;
    char station_name[50];
} Neighbor;

typedef struct {
    char departure_time[6];
    char bus_number[10];
    char departure_stop[50];
    char destination_station[50];
    char arrival_time[6];
} BusSchedule;

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

void *tcp_thread_func(void *arg);
void *udp_thread_func(void *arg);
void read_timetable(const char *station_name);

#endif // SERVER_H
