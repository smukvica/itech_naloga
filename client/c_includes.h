#pragma once

typedef struct package{
    char *data;
    unsigned int status;
}package;

typedef struct arg_struct{
}arg_struct;

typedef struct parameters{
    int queue_size;
    int number_of_packets;
    int number_of_bpm;
    int file_entries;
    int num_of_fields;
    int size_of_field;
    char names[10][32];
    int file_write;
    int std_output;
    int port;
    char ip[15];
}parameters;