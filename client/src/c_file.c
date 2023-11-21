#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "c_file.h"
#include "c_queue.h"


// loads parameters from file
void load_params(const char *a_file, parameters *a_params){
    FILE *f;
    f = fopen(a_file, "rb");
    char *line = NULL;
    ssize_t read;
    size_t len = 0;
    char tf_values[2][6];
    char names[MAX_NAMES*MAX_NAME_LENGTH] = {0};
    // if file doesn't exist don't read
    if(f != NULL){
        while ((read = getline(&line, &len, f)) != -1) {
            sscanf(line, "number_of_fields: %d", &a_params->number_of_fields);
            sscanf(line, "size_of_field: %d", &a_params->size_of_field);
            sscanf(line, "queue_size: %d", &a_params->queue_size);
            sscanf(line, "number_of_packets: %d", &a_params->number_of_packets);
            sscanf(line, "number_of_bpm: %d", &a_params->number_of_bpm);
            sscanf(line, "file_entries: %d", &a_params->file_entries);
            sscanf(line, "ip: %d.%d.%d.%d", &a_params->ip[0], &a_params->ip[1], 
                                            &a_params->ip[2], &a_params->ip[3]);
            sscanf(line, "port: %d", &a_params->port);
            sscanf(line, "output: %s", tf_values[0]);
            sscanf(line, "writer: %s", tf_values[1]);
            sscanf(line, "folder: %s", &a_params->save_folder[0]);
            sscanf(line, "names: %329c", names);
        }

        int i = 0;
        char *temp;
        temp = strtok(names, "\n");
        temp = strtok(names, " ");
        while(temp != NULL){
            if(strlen(temp) >= MAX_NAME_LENGTH)
                temp[31] = '\0';
            strcpy(a_params->names[i], temp);
            temp = strtok(NULL, " ");
            i++;
        }
        
        int param_error = 0;

        if(strcmp(tf_values[0], "true") == 0)
            a_params->std_output = true;
        else if(strcmp(tf_values[0], "false") == 0)
            a_params->std_output = false;
        else
            param_error = 1;
        if(strcmp(tf_values[1], "true") == 0)
            a_params->file_write = true;
        else if(strcmp(tf_values[1], "false") == 0)
            a_params->file_write = false;
        else
            param_error = 1;
        
        if(check_parameter_limits("number_of_fields", a_params->number_of_fields)){
            param_error = 1;
        }
        if(check_parameter_limits("size_of_field", a_params->size_of_field)){
            param_error = 1;
        }
        if(check_parameter_limits("queue_size", a_params->queue_size)){
            param_error = 1;
        }
        if(check_parameter_limits("number_of_bpm", a_params->number_of_bpm)){
            param_error = 1;
        }
        if(check_parameter_limits("file_entries", a_params->file_entries)){
            param_error = 1;
        }
        if(param_error)
            printf("Error in config file. See /help.\n");
        fclose(f);
    }
}

// writer thread function
void *file_writer(void *a_args){
    FILE *f;
    parameters *params = a_args;
    // write parameters without file_write
    parameters temp;
    memcpy(&temp, params, sizeof(parameters));
    temp.file_write = false;
    char filename[512];  // save file
    int file_num = 0;   // current file number
    char data[(get_limit("number_of_fields", 1) * 
               get_limit("size_of_field", 1) + STATUS_FIELD_SIZE) * 
               get_limit("file_entries", 1)];
    
    int size_of_data = (params->number_of_fields * 
                        params->size_of_field + STATUS_FIELD_SIZE);
    mkdir(params->save_folder, 0777);
    while(1){
        // gets data from queue
        int ret = get_from_queue(&data[0], params->file_entries, FILEW, *params);
        
        // if no data was aquired keep trying
        while(ret){
            ret = get_from_queue(&data[0], params->file_entries, FILEW, *params);
            if(get_program_terminate() == 1){
                printf("terminate writer\n");
                return 0;
            }
            sleep(0);
        }
        if(get_trace() == 1)
            printf("writing to file\n");
        if(params->file_write == true){
            // data aquired set filename, open file and write to it
            sprintf(filename, "%sfile_%05d.bin", params->save_folder, file_num);
            f = fopen(filename,"wb");
            fwrite(&temp, sizeof(parameters) - member_size(parameters, save_folder), 1, f);
            fwrite(data, size_of_data, params->file_entries, f);
            fclose(f);

            file_num++;
        }
    }
}

// read from file
void file_reader(const char *a_file, parameters *a_params){
    if(get_trace() == 1)
        printf("reading from file\n");
    FILE *f;
    f = fopen(a_file, "rb");
    if(f == NULL){
        printf("no file found\n");
        return;
    }

    int size_of_data = (a_params->number_of_fields * 
                        a_params->size_of_field + STATUS_FIELD_SIZE);

    fread(a_params, sizeof(parameters) - member_size(parameters, save_folder), 1, f);
    setup_queue(*a_params);

    char buffer[ size_of_data * a_params->file_entries];

    fread(buffer, size_of_data, a_params->file_entries, f);

    // write data to queue
    write_to_queue(buffer, a_params->file_entries, FILEW, *a_params);

    fclose(f);
}