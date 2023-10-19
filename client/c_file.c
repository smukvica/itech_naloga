#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "c_includes.h"
#include "c_file.h"

extern char *file_queue[2];
extern int num_of_fields;
extern int size_of_field;
extern int file_entries;

extern sem_t semaphore_file;
extern int switch_buffer;
int local_switch_buffer = 0;
int file_buffer_f = 0;

extern char **names;

extern int program_terminate;

void save_params(){
    FILE *f;
    f = fopen("params.txt", "w");

    fprintf(f, "fields: %d", num_of_fields);
    fprintf(f, "\nsize of field: %d", size_of_field);
    fprintf(f, "\nfile entries: %d", file_entries);

    fprintf(f, "\nfield names: ");
    for(int i = 0; i < num_of_fields; i++){
        fprintf(f, "%s ", names[i]);
    }
    
    fclose(f);
}

void load_params(const char *filename){
    FILE *f;
    f = fopen(filename, "r");

    char *line = NULL;
    size_t len = 0;

    int read = getline(&line, &len, f);
    sscanf(line, "fields: %d", &num_of_fields);
    read = getline(&line, &len, f);
    sscanf(line, "size of field: %d", &size_of_field);
    read = getline(&line, &len, f);
    sscanf(line, "file entries: %d", &file_entries);
    read = getline(&line, &len, f);

    names = malloc(sizeof(char*) * num_of_fields);
    for(int i = 0; i < num_of_fields; i++){
        names[i] = malloc(sizeof(char) * 10);
    }

    char *token = strtok(line, " ");
    for(int k = 1; k <= num_of_fields + 1; k++){
        token = strtok(NULL, " ");
        if(k - 2 >= 0)
            strcpy(names[k - 2], token);
    }

    fclose(f);
}

void *file_writer(void *arguments){
    FILE *write;

    int i = 0;
    char filename[20];
    int file_num = 0;
    while(1){
        sprintf(filename, "file_%05d.bin", file_num);
        write = fopen(filename,"wb");

        while(local_switch_buffer != 1){
            if(program_terminate == 1){
                printf("terminate writer\n");
                return 0;
            }
            sem_wait(&semaphore_file);
            local_switch_buffer = switch_buffer;
            sem_post(&semaphore_file);
            sleep(0);
        }

        fwrite(file_queue[file_buffer_f], (sizeof(char) * num_of_fields * size_of_field + 4), file_entries, write);
        file_buffer_f = (file_buffer_f + 1) % 2;
        local_switch_buffer = 0;
        sem_wait(&semaphore_file);
        switch_buffer = 0;
        sem_post(&semaphore_file);
        fclose(write);
        i += file_entries;
        file_num++;
    }
}