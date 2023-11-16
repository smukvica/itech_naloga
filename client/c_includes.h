#pragma once
#include <stdbool.h>

#define member_size(type, member) sizeof(((type *)0)->member)


// returns 0 if parameter is within limits
// returns 1 if parameter is out of limits
int check_parameter_limits(const char *parameter, int value);

extern int trace;

typedef struct parameters{
    int queue_size;
    int number_of_packets;
    int number_of_bpm;
    int file_entries;
    int number_of_fields;
    int size_of_field;
    char names[10][32];
    bool file_write;
    bool std_output;
    int port;
    int ip[4];
    char save_folder[256];
}parameters;

typedef struct limits{
    int queue_size[2];
    int number_of_bpm[2];
    int file_entries[2];
    int number_of_fields[2];
    int size_of_field[2];
}limits;

static const limits param_limits = {
    .queue_size = {200000, 100000000},
    .number_of_bpm = {1, 4},
    .file_entries = {500, 10000},
    .number_of_fields = {1, 10},
    .size_of_field = {1, 4}
};