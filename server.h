#ifndef SERVER_H
#define SERVER_H

#include <json-c/json.h>

#define BUF_SIZE 1024
#define BUFFER_SIZE 1024
#define INITIAL_SCHEDULE_CAPACITY 10

typedef struct {
    char ip[50];
    int udp_port;
    char station_name[50]; 
} Neighbor;

typedef struct {
    char departure_time[6];
    char bus_number[10];
    char departure_stop[50];
    char arrival_time[6];
    char destination_station[50];
} BusSchedule;

typedef struct {
    int socket;
    char station_name[50];
    Neighbor *neighbors;
    int neighbor_count;
} ThreadArgs;

// Global schedule array and counter
extern BusSchedule *schedule_array;
extern int schedule_count;
extern int schedule_capacity;

// Function declarations
void printSchedule(const BusSchedule *schedule);
void printAllSchedules();
void read_timetable(const char *station_name);
void *tcp_thread_func(void *arg);
void *udp_thread_func(void *arg);
int create_and_bind_socket(int port, int type);

#endif
