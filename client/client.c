#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "c_includes.h"
#include "c_receiver.h"
#include "c_output.h"
#include "c_file.h"

int queue_size = 100;
int number_of_packets = 1000;
int number_of_bpm = 1;
int file_entries = 100;
int num_of_fields = 3;
int size_of_field = 4;

char **names;

sem_t semaphore_output;
sem_t semaphore_file;


package *queue;
int current_q_w = 0;
int current_q_r = 0;
int current_limit = 0;
int q_overflow = 0;



pthread_t receiver;
pthread_t output;
pthread_t file;

char *file_queue[2];
int switch_buffer = 0;

int program_terminate = 0;

void setup_queue_memory(){
    queue = malloc(sizeof(package) * queue_size);
    for(int i = 0; i < queue_size; i++){
        queue[i].data = malloc(sizeof(char) * size_of_field * num_of_fields);
    }
}

void free_queue_memory(){
    for(int i = 0; i < queue_size; i++){
        free(queue[i].data);
    }
    free(queue);
}

void setup_names_memory(int f){
    names = malloc(sizeof(char*) * f);
    for(int i = 0; i < f; i++){
        names[i] = malloc(sizeof(char) * 10);
    }
}

void free_names_memory(){
    for(int i = 0; i < num_of_fields; i++){
        free(names[i]);
    }
    free(names);
}

void setup_file_memory(){
    file_queue[0] = malloc((sizeof(char) * size_of_field * num_of_fields + 4) * file_entries);

    file_queue[1] = malloc((sizeof(char) * size_of_field * num_of_fields + 4) * file_entries);
}

void free_file_memory(){
    free(file_queue[0]);
    free(file_queue[1]);
}

void read_file(const char *file){
    FILE *write;
    write = fopen(file,"rb");  // r for read, b for binary

    load_params("params.txt");

    setup_queue_memory();

    char buffer[num_of_fields * size_of_field + 4];

    for(int i = 0; i < 100; i++){
        queue[i].data = malloc(num_of_fields * size_of_field);
        fread(buffer, sizeof(char) * num_of_fields * size_of_field + 4, 1,write);
        memcpy(queue[i].data, buffer, num_of_fields * size_of_field);
        memcpy(&queue[i].status, buffer + num_of_fields * size_of_field, 4);
    }

    output_from_file(file_entries);

    free_queue_memory();
    free_names_memory();
}

void sigint_handler(){
    program_terminate = 1;
}

int main(int argc , char *argv[])
{

    /*
        arguments:      name value
            queue_size
            number_of_packets
            number_of_bpm
            file_entries
    */

    signal(SIGINT, sigint_handler);

    if(strcmp(argv[1], "read_file") == 0){
        read_file(argv[2]);
        return 0;
    }

    int c = 1;
    while(c < argc){
        if(strcmp(argv[c], "structure") == 0){
            int f,s;
            sscanf(argv[c+1], "%dx%d", &f, &s);
            setup_names_memory(f);
            for(int k = 0; k < f; k++){
                strcpy(names[k], argv[c+2+k]);
            }
            c += f;
            num_of_fields = f;
            size_of_field = s;
        }
        if(strcmp(argv[c], "queue_size") == 0){
            queue_size = atoi(argv[c+1]);
        }
        if(strcmp(argv[c], "number_of_packets") == 0){
            number_of_packets = atoi(argv[c+1]);
        }
        if(strcmp(argv[c], "number_of_bpm") == 0){
            number_of_bpm = atoi(argv[c+1]);
        }
        if(strcmp(argv[c], "file_entries") == 0){
            file_entries = atoi(argv[c+1]);
        }
        c += 2;
    }

    save_params();

    setup_queue_memory();
    setup_file_memory();

    sem_init(&semaphore_output, 0, 1);
    sem_init(&semaphore_file, 0, 1);

    pthread_create(&receiver, NULL, read_package, NULL);
    pthread_create(&output, NULL, output_package, NULL);
    pthread_create(&file, NULL, file_write, NULL);
    

    while(program_terminate != 1){

    }

    pthread_join(output, NULL);
    pthread_join(receiver, NULL);
    pthread_join(file, NULL);
	
    free_queue_memory();
    free_names_memory();
    free_file_memory();

	return 0;
}



/*

compile:
gcc *.c -lpthread -o client

run:
./client read_file "filename" # reading a single file and outputing
./client structure 3x4 A B C queue_size 100 number_of_bpm 1 file_entries 100 # any parameter can be skipped except structure


stop:
Ctrl + C


*/