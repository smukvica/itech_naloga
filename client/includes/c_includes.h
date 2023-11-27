#pragma once
#include <stdbool.h>

#define MAX_NAMES 10
#define MAX_NAME_LENGTH 32
#define SAVE_FOLDER_NAME_LENGTH 256
#define SAVE_FILE_NAME_LENGTH 100

#define member_size(type, member) sizeof(((type *)0)->member)

typedef struct parameters{
    int queue_size;
    int number_of_packets;
    int number_of_bpm;
    int file_entries;
    int number_of_fields;
    int size_of_field;
    char names[MAX_NAMES][MAX_NAME_LENGTH];
    bool file_write;
    bool std_output;
    bool check_status;
    int status_field_size;
    int port;
    char save_folder[SAVE_FOLDER_NAME_LENGTH];
}parameters;

extern char g_filename[SAVE_FILE_NAME_LENGTH];

// returns 0 if parameter is within limits
// returns 1 if parameter is out of limits
int check_parameter_limits(const char *a_parameter, int a_value);
int get_limit(const char *a_parameter, int a_lower_upper);

// get/set for global variables
int get_program_terminate();
int get_setup_complete();
int get_read_file();
int get_start_stop();
int get_trace();

void set_program_terminate(int a_v);
void set_setup_complete(int a_v);
void set_read_file(int a_v);
void set_start_stop(int ta_v);
void set_trace(int a_v);

void setup_includes();
void free_includes();