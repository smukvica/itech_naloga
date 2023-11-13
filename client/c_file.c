#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "c_file.h"
#include "c_queue.h"

extern int program_terminate;
extern char save_folder[256];

// saves parameters to file
void save_params(parameters params){
    FILE *f;
    f = fopen("params.bin", "wb");
    fwrite(&params, sizeof(parameters), 1, f);
    fclose(f);
}

// loads parameters from file
void load_params(parameters *params){
    FILE *f;
    f = fopen("config", "rb");
    char *line = NULL;
    ssize_t read;
    size_t len = 0;
    char tf_values[2][5];
    // if file doesn't exist don't read
    if(f != NULL){
        while ((read = getline(&line, &len, f)) != -1) {
            sscanf(line, "number_of_fields: %d", &params->number_of_fields);
            sscanf(line, "size_of_field: %d", &params->size_of_field);
            sscanf(line, "queue_size: %d", &params->queue_size);
            sscanf(line, "number_of_packets: %d", &params->number_of_packets);
            sscanf(line, "number_of_bpm: %d", &params->number_of_bpm);
            sscanf(line, "file_entries: %d", &params->file_entries);
            sscanf(line, "ip: %d.%d.%d.%d", &params->ip[0], &params->ip[1], 
                                            &params->ip[2], &params->ip[3]);
            sscanf(line, "port: %d", &params->port);
            sscanf(line, "output: %s", tf_values[0]);
            sscanf(line, "writer: %s", tf_values[1]);
            sscanf(line, "folder: %s", &save_folder[0]);
        }
        int param_error = 0;

        if(strcmp(tf_values[0], "true") == 0)
            params->std_output = true;
        else if(strcmp(tf_values[0], "false") == 0)
            params->std_output = false;
        else
            param_error = 1;
        if(strcmp(tf_values[1], "true") == 0)
            params->file_write = true;
        else if(strcmp(tf_values[1], "false") == 0)
            params->file_write = false;
        else 
            param_error = 1;
        
        if(params->number_of_fields < 1 || params->number_of_fields > 10){
            param_error = 1;
        }
        if(params->size_of_field != 1 && params->size_of_field != 2 && 
           params->size_of_field != 4){
            param_error = 1;
        }
        if(params->queue_size < 200000 || params->queue_size > 100000000){
            param_error = 1;
        }
        if(params->number_of_bpm < 1 || params->number_of_bpm > 4){
            param_error = 1;
        }
        if(param_error)
            printf("Error in config file. See /help.\n");
        fclose(f);
    }
}

// writer thread function
void *file_writer(void *args){
    FILE *f;
    parameters params = *(parameters *)args;
    // write parameters without file_write
    parameters temp;
    memcpy(&temp, &params, sizeof(parameters));
    temp.file_write = false;
    char filename[512];  // save file
    int file_num = 0;   // current file number
    char data[(params.number_of_fields * params.size_of_field + 4) * 
               params.file_entries];
    
    mkdir(save_folder, 0777);
    while(1){
        // gets data from queue
        int ret = get_from_queue(&data[0], params.file_entries, FILEW, params);
        
        // if no data was aquired keep trying
        while(ret){
            ret = get_from_queue(&data[0], params.file_entries, FILEW, params);
            if(program_terminate == 1){
                printf("terminate writer\n");
                return 0;
            }
            sleep(0);
        }
        // data aquired set filename, open file and write to it
        sprintf(filename, "%s/file_%05d.bin", save_folder, file_num);
        f = fopen(filename,"wb");
        fwrite(&temp, sizeof(parameters), 1, f);
        fwrite(data, 
               (sizeof(char) * params.number_of_fields * params.size_of_field + 4), 
               params.file_entries, 
               f);
        fclose(f);

        file_num++;
    }
}

// read from file
void file_reader(const char *file, parameters *params){
    FILE *f;
    f = fopen(file, "rb");
    if(f == NULL){
        printf("no file found\n");
        return;
    }

    fread(params, sizeof(parameters), 1, f);
    setup_queue(*params);

    char buffer[(params->number_of_fields * params->size_of_field + 4) * 
                 params->file_entries];
    
    fread(buffer, (sizeof(char) * params->number_of_fields * 
                   params->size_of_field + 4), params->file_entries, f);

    // write data to queue
    write_to_queue(buffer, params->file_entries, FILEW, *params);

    fclose(f);
}