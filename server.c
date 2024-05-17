#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "server.h"

BusSchedule *schedule_array = NULL;
int schedule_count = 0;
int schedule_capacity = 20;

void printSchedule(const BusSchedule* schedule) {
    printf("Departure: %s, Bus No: %s, Departure Stop: %s, Arrival: %s, Destination: %s\n",
           schedule->departure_time,
           schedule->bus_number,
           schedule->departure_stop,
           schedule->arrival_time,
           schedule->destination_station);
}

void printAllSchedules() {
    printf("Printing all schedules:\n");
    for (int i = 0; i < schedule_count; i++) {
        printSchedule(&schedule_array[i]);
    }
}

void read_timetable(const char *station_name) {
    char filename[256];
    snprintf(filename, sizeof(filename), "tt-%s", station_name);

    printf("Attempting to open file: %s\n", filename);

    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open timetable file");
        printf("Ensure that the file %s exists in the current working directory and has the correct permissions.\n", filename);
        exit(EXIT_FAILURE);
    }

    schedule_array = malloc(schedule_capacity * sizeof(BusSchedule));
    if (schedule_array == NULL) {
        perror("Failed to allocate memory for schedule array");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    while (fgets(buffer, BUFFER_SIZE, file)) {
        if (buffer[0] == '#') {
            continue;
        }

        if (schedule_count >= schedule_capacity) {
            schedule_capacity *= 2;
            BusSchedule *new_array = realloc(schedule_array, schedule_capacity * sizeof(BusSchedule));
            if (new_array == NULL) {
                perror("Failed to reallocate memory for schedule array");
                free(schedule_array);
                exit(EXIT_FAILURE);
            }
            schedule_array = new_array;
        }

        BusSchedule schedule;
        char *token = strtok(buffer, ",");
        if (token == NULL) { continue; }
        strncpy(schedule.departure_time, token, sizeof(schedule.departure_time) - 1);
        schedule.departure_time[sizeof(schedule.departure_time) - 1] = '\0';

        token = strtok(NULL, ",");
        if (token == NULL) { continue; }
        strncpy(schedule.bus_number, token, sizeof(schedule.bus_number) - 1);
        schedule.bus_number[sizeof(schedule.bus_number) - 1] = '\0';

        token = strtok(NULL, ",");
        if (token == NULL) { continue; }
        strncpy(schedule.departure_stop, token, sizeof(schedule.departure_stop) - 1);
        schedule.departure_stop[sizeof(schedule.departure_stop) - 1] = '\0';

        token = strtok(NULL, ",");
        if (token == NULL) { continue; }
        strncpy(schedule.arrival_time, token, sizeof(schedule.arrival_time) - 1);
        schedule.arrival_time[sizeof(schedule.arrival_time) - 1] = '\0';

        token = strtok(NULL, "\n");
        if (token == NULL) { continue; }
        strncpy(schedule.destination_station, token, sizeof(schedule.destination_station) - 1);
        schedule.destination_station[sizeof(schedule.destination_station) - 1] = '\0';

        schedule_array[schedule_count++] = schedule;
    }

    fclose(file);

    printAllSchedules();
}
