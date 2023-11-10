#pragma once

typedef struct parameters{
    int queue_size;
    int number_of_packets;
    int number_of_bpm;
    int file_entries;
    int number_of_fields;
    int size_of_field;
    char names[10][32];
    int file_write;
    int std_output;
    int port;
    char ip[15];
}parameters;