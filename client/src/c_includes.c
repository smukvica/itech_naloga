#include <string.h>

#include "c_includes.h"

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

int check_parameter_limits(const char *parameter, int value){
    if(strcmp(parameter, "number_of_fields") == 0)
        if(value < get_limit("number_of_fields", 0) || 
           value > get_limit("number_of_fields", 1))
            return 1;
    else if(strcmp(parameter, "size_of_field") == 0)
        if(value < get_limit("size_of_field", 0) ||
           value > get_limit("size_of_field", 1))
            return 1;
    else if(strcmp(parameter, "queue_size") == 0)
        if(value < get_limit("queue_size", 0) || 
           value > get_limit("queue_size", 1))
            return 1;
    else if(strcmp(parameter, "number_of_bpm") == 0)
        if(value < get_limit("number_of_bpm", 0) || 
           value > get_limit("number_of_bpm", 1))
            return 1;
    else if(strcmp(parameter, "file_entries") == 0)
        if(value < get_limit("file_entries", 0) || 
           value > get_limit("file_entries", 1))
            return 1;
    return 0;
}

int get_limit(const char *parameter, int lower_upper){
    if(strcmp(parameter, "number_of_fields") == 0)
        return param_limits.number_of_fields[lower_upper];
    else if(strcmp(parameter, "size_of_field") == 0)
        return param_limits.size_of_field[lower_upper];
    else if(strcmp(parameter, "queue_size") == 0)
        return param_limits.queue_size[lower_upper];
    else if(strcmp(parameter, "number_of_bpm") == 0)
        return param_limits.number_of_bpm[lower_upper];
    else if(strcmp(parameter, "file_entries") == 0)
        return param_limits.file_entries[lower_upper];
    return 0;
}