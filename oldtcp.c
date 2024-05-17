#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <uuid/uuid.h>
#include <json-c/json.h>
#include "server.h"

void handle_tcp(int tcp_sock, const char *station_name, Neighbor *neighbors, int neighbor_count, BusSchedule *schedule_array, int schedule_count) {
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

        json_object *jobj = json_object_new_object();
        json_object_object_add(jobj, "id", json_object_new_string(uuid_str));
        json_object_object_add(jobj, "origin", json_object_new_string(departure_stop));
        json_object_object_add(jobj, "destination", json_object_new_string(destination_station));
        json_object_object_add(jobj, "delivered", json_object_new_boolean(0));
        json_object *route_array = json_object_new_array();

        BusSchedule *selected_schedule = NULL;
        for (int i = 0; i < schedule_count; i++) {
            if (strcmp(schedule_array[i].departure_stop, departure_stop) == 0 &&
                strcmp(schedule_array[i].destination_station, destination_station) == 0 &&
                strcmp(schedule_array[i].departure_time, start_time) >= 0) {
                if (selected_schedule == NULL || strcmp(schedule_array[i].departure_time, selected_schedule->departure_time) < 0) {
                    selected_schedule = &schedule_array[i];
                }
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
            json_object *neighbors_array = json_object_new_array();
            for (int i = 0; i < neighbor_count; i++) {
                json_object *neighbor_obj = json_object_new_object();
                json_object_object_add(neighbor_obj, "name", json_object_new_string(neighbors[i].station_name));
                json_object_object_add(neighbor_obj, "ip", json_object_new_string(neighbors[i].ip));
                json_object_object_add(neighbor_obj, "port", json_object_new_int(neighbors[i].udp_port));
                json_object_array_add(neighbors_array, neighbor_obj);
            }
            json_object_object_add(jobj, "neighbors", neighbors_array);
        }

        json_object_object_add(jobj, "route", route_array);

        const char *json_str = json_object_to_json_string(jobj);

        printf("Generated JSON: %s\n", json_str);

        char http_response[BUF_SIZE];
        snprintf(http_response, sizeof(http_response),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: application/json\r\n"
                 "Content-Length: %lu\r\n"
                 "\r\n"
                 "%s",
                 strlen(json_str), json_str);

        send(new_socket, http_response, strlen(http_response), 0);

        json_object_put(jobj);
        close(new_socket);
    }
}

void *tcp_thread_func(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    handle_tcp(args->socket, arg
}