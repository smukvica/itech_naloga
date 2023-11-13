#pragma once
#include <stdbool.h>

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
}parameters;

typedef struct limits{
    int queue_size[2];
    int number_of_packets[2];
    int number_of_bpm[2];
    int file_entries[2];
    int number_of_fields[2];
    int size_of_field[2];
}limits;

extern const limits param_limits;