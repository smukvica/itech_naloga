#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "c_file.h"
#include "c_queue.h"

extern int program_terminate;

void save_params(parameters params){
    FILE *f;
    f = fopen("params.txt", "w");

    fprintf(f, "fields: %d", params.num_of_fields);
    fprintf(f, "\nsize of field: %d", params.size_of_field);
    fprintf(f, "\nfile entries: %d", params.file_entries);

    fprintf(f, "\nfield names: ");
    for(int i = 0; i < params.num_of_fields; i++){
        fprintf(f, "%s ", params.names[i]);
    }
    
    fclose(f);
}

void load_params(const char *filename, parameters *params){
    FILE *f;
    f = fopen(filename, "r");

    char *line = NULL;
    size_t len = 0;

    int read = getline(&line, &len, f);
    sscanf(line, "fields: %d", &params->num_of_fields);
    read = getline(&line, &len, f);
    sscanf(line, "size of field: %d", &params->size_of_field);
    read = getline(&line, &len, f);
    sscanf(line, "file entries: %d", &params->file_entries);
    read = getline(&line, &len, f);

    char *token = strtok(line, " ");
    for(int k = 1; k <= params->num_of_fields + 1; k++){
        token = strtok(NULL, " ");
        if(k - 2 >= 0)
            strcpy(params->names[k - 2], token);
    }

    fclose(f);

    params->queue_size = params->file_entries;
}

void *file_writer(void *args){
    FILE *f;
    parameters params = *(parameters *)args;
    char filename[20];
    int file_num = 0;
    char data[(params.num_of_fields * params.size_of_field + 4) * params.file_entries];
    while(1){
        int ret = get_from_queue(&data[0], params.file_entries, FILEW, params);
        
        while(ret){
            ret = get_from_queue(&data[0], params.file_entries, FILEW, params);
            if(program_terminate == 1){
                printf("terminate writer\n");
                return 0;
            }
            sleep(0);
        }

        sprintf(filename, "file_%05d.bin", file_num);
        f = fopen(filename,"wb");
        fwrite(data, (sizeof(char) * params.num_of_fields * params.size_of_field + 4), params.file_entries, f);
        fclose(f);

        file_num++;
    }
}

void file_reader(const char *file, parameters params){
    FILE *f;
    f = fopen(file, "rb");

    char buffer[(params.num_of_fields * params.size_of_field + 4) * params.file_entries];
    
    fread(buffer, sizeof(char) * params.num_of_fields * params.size_of_field + 4 , params.file_entries, f);
    write_to_queue(&buffer[0], params.file_entries, FILEW, params);

    fclose(f);
}