#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <uuid/uuid.h>
#include <pthread.h>
#include "server.h"
#include <json-c/json.h>

#define BUF_SIZE 1024

void handle_tcp(int tcp_sock, const char *station_name, Neighbor *neighbors, int neighbor_count, BusSchedule *schedule_array, int schedule_count, pthread_mutex_t *mutex) {
    char buffer[BUF_SIZE];
    int new_socket;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    uuid_t binuuid;
    char uuid_str[37];

    while ((new_socket = accept(tcp_sock, (struct sockaddr *)&client_addr, &addr_len)) >= 0) {
        memset(buffer, 0, BUF_SIZE);
        read(new_socket, buffer, BUF_SIZE);

        char start_time[6], departure_stop[50], destination_station[50];
        sscanf(buffer, "GET /?start_time=%5[^&]&departure_stop=%49[^&]&destination_station=%49[^ ]", start_time, departure_stop, destination_station);

        uuid_generate_random(binuuid);
        uuid_unparse_lower(binuuid, uuid_str);

        printf("Parsed HTTP Request: start_time=%s, departure_stop=%s, destination_station=%s\n", start_time, departure_stop, destination_station);

        pthread_mutex_lock(mutex);
        printf("Number of Neighbors: %d\n", neighbor_count);
        for (int i = 0; i < neighbor_count; i++) {
            printf("Neighbor %d: IP=%s, Port=%d, Station Name=%s\n", i + 1, neighbors[i].ip, neighbors[i].udp_port, neighbors[i].station_name);
        }
        pthread_mutex_unlock(mutex);

        json_object *jobj = json_object_new_object();
        json_object_object_add(jobj, "id", json_object_new_string(uuid_str));
        json_object_object_add(jobj, "source", json_object_new_string(departure_stop));
        json_object_object_add(jobj, "destination", json_object_new_string(destination_station));
        json_object_object_add(jobj, "returning", json_object_new_boolean(0));
        json_object *route_array = json_object_new_array();
        json_object_object_add(jobj, "route", route_array);

        BusSchedule *selected_schedule = NULL;
        for (int i = 0; i < schedule_count; i++) {
            if (strcmp(schedule_array[i].destination_station, destination_station) == 0 && strcmp(schedule_array[i].departure_time, start_time) > 0) {
                selected_schedule = &schedule_array[i];
                break;
            }
        }

        if (selected_schedule != NULL) {
            json_object *journey_obj = json_object_new_object();
            json_object_object_add(journey_obj, "depart_location", json_object_new_string(selected_schedule->departure_stop));
            json_object_object_add(journey_obj, "depart_time", json_object_new_string(selected_schedule->departure_time));
            json_object_object_add(journey_obj, "arrival_location", json_object_new_string(selected_schedule->destination_station));
            json_object_object_add(journey_obj, "arrival_time", json_object_new_string(selected_schedule->arrival_time));
            json_object_array_add(route_array, journey_obj);

            json_object_object_add(jobj, "delivered", json_object_new_boolean(1));
        } else {
            json_object_object_add(jobj, "delivered", json_object_new_boolean(0));
        }

        // Convert JSON object to string
        const char *json_str = json_object_to_json_string(jobj);

        // Debugging output to print the JSON packet
        printf("Generated JSON: %s\n", json_str);

        // Send the JSON string as an HTTP response
        char http_response[BUF_SIZE];
        snprintf(http_response, sizeof(http_response),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: application/json\r\n"
                 "Content-Length: %lu\r\n"
                 "\r\n"
                 "%s",
                 strlen(json_str), json_str);

        send(new_socket, http_response, strlen(http_response), 0);

        // Clean up
        json_object_put(jobj);
        close(new_socket);
    }
}

void *tcp_thread_func(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    handle_tcp(args->socket, args->station_name, args->neighbors, args->neighbor_count, args->schedule_array, args->schedule_count, args->mutex);
    return NULL;
}
