#include <string.h>

#include "c_includes.h"

int check_parameter_limits(const char *parameter, int value){
    if(strcmp(parameter, "number_of_fields") == 0)
        if(value < param_limits.number_of_fields[0] || 
           value > param_limits.number_of_fields[1])
            return 1;
    else if(strcmp(parameter, "size_of_field") == 0)
        if(value < param_limits.size_of_field[0] ||
           value > param_limits.size_of_field[1])
            return 1;
    else if(strcmp(parameter, "queue_size") == 0)
        if(value < param_limits.queue_size[0] || 
           value > param_limits.queue_size[1])
            return 1;
    else if(strcmp(parameter, "number_of_bpm") == 0)
        if(value < param_limits.number_of_bpm[0] || 
           value > param_limits.number_of_bpm[1])
            return 1;
    else if(strcmp(parameter, "file_entries") == 0)
        if(value < param_limits.file_entries[0] || 
           value > param_limits.file_entries[1])
            return 1;
    return 0;
}