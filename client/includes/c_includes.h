#pragma once
#include <stdbool.h>

#define member_size(type, member) sizeof(((type *)0)->member)


// returns 0 if parameter is within limits
// returns 1 if parameter is out of limits
int check_parameter_limits(const char *a_parameter, int a_value);
int get_limit(const char *a_parameter, int a_lower_upper);

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