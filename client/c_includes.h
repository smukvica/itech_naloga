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