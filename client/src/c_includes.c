#include <string.h>
#include <semaphore.h>

#include "c_includes.h"

int g_program_terminate = 0;
int g_setup_complete = 0;
int g_read_file = 0;
int g_start_stop = 0;
int g_trace = 0;

char g_filename[100];

sem_t semaphore_g;

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

int check_parameter_limits(const char *a_parameter, int a_value){
    if(strcmp(a_parameter, "number_of_fields") == 0)
        if(a_value < get_limit("number_of_fields", 0) || 
           a_value > get_limit("number_of_fields", 1))
            return 1;
    else if(strcmp(a_parameter, "size_of_field") == 0)
        if(a_value < get_limit("size_of_field", 0) ||
           a_value > get_limit("size_of_field", 1))
            return 1;
    else if(strcmp(a_parameter, "queue_size") == 0)
        if(a_value < get_limit("queue_size", 0) || 
           a_value > get_limit("queue_size", 1))
            return 1;
    else if(strcmp(a_parameter, "number_of_bpm") == 0)
        if(a_value < get_limit("number_of_bpm", 0) || 
           a_value > get_limit("number_of_bpm", 1))
            return 1;
    else if(strcmp(a_parameter, "file_entries") == 0)
        if(a_value < get_limit("file_entries", 0) || 
           a_value > get_limit("file_entries", 1))
            return 1;
    return 0;
}

int get_limit(const char *a_parameter, int a_lower_upper){
    if(strcmp(a_parameter, "number_of_fields") == 0)
        return param_limits.number_of_fields[a_lower_upper];
    else if(strcmp(a_parameter, "size_of_field") == 0)
        return param_limits.size_of_field[a_lower_upper];
    else if(strcmp(a_parameter, "queue_size") == 0)
        return param_limits.queue_size[a_lower_upper];
    else if(strcmp(a_parameter, "number_of_bpm") == 0)
        return param_limits.number_of_bpm[a_lower_upper];
    else if(strcmp(a_parameter, "file_entries") == 0)
        return param_limits.file_entries[a_lower_upper];
    return 0;
}

int get_program_terminate() {
    sem_wait(&semaphore_g);
    int r = g_program_terminate;
    sem_post(&semaphore_g);
    return r;
}
int get_setup_complete() {
    int r = g_setup_complete;
    return r;
}
int get_read_file() {
    sem_wait(&semaphore_g);
    int r = g_read_file;
    sem_post(&semaphore_g);
    return r;
}
int get_start_stop() {
    int r = g_start_stop;
    return r;
}
int get_trace() {
    int r = g_start_stop;
    return r;
}

void set_program_terminate(int a_v) {
    sem_wait(&semaphore_g);
    g_program_terminate = a_v;
    sem_post(&semaphore_g);
}
void set_setup_complete(int a_v) {
    g_setup_complete = a_v;
}
void set_read_file(int a_v) {
    sem_wait(&semaphore_g);
    g_read_file = a_v;
    sem_post(&semaphore_g);
}
void set_start_stop(int a_v) {
    g_start_stop = a_v;
}
void set_trace(int a_v) {
    g_trace = a_v;
}

void setup_includes(){
    sem_init(&semaphore_g, 0, 1);
}

void free_includes(){
    sem_destroy(&semaphore_g);
}