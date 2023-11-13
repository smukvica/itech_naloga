#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "c_file.h"
#include "c_queue.h"

extern int program_terminate;
extern char save_folder[64];

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
    f = fopen("params.bin", "rb");
    // if file doesn't exist don't read
    if(f != NULL){
        fread(params, sizeof(parameters), 1, f);
        fclose(f);
    }
}

// writer thread function
void *file_writer(void *args){
    FILE *f;
    parameters params = *(parameters *)args;
    char filename[100];  // save file
    int file_num = 0;   // current file number
    char data[(params.number_of_fields * params.size_of_field + 4) * params.file_entries];
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
        fwrite(&params, sizeof(parameters), 1, f);
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

    char buffer[(params->number_of_fields * params->size_of_field + 4) * 
                 params->file_entries];
    
    fread(buffer, (sizeof(char) * params->number_of_fields * 
                   params->size_of_field + 4), params->file_entries, f);

    // write data to queue
    write_to_queue(buffer, params->file_entries, FILEW, *params);

    fclose(f);
}