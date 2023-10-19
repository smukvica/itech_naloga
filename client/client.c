#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "c_includes.h"
#include "c_receiver.h"
#include "c_output.h"

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

extern package file_queue[2];

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

int main(int argc , char *argv[])
{

    /*
        arguments:      name value
            queue_size
            number_of_packets
            number_of_bpm
            file_entries
    */
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

    setup_queue_memory();

    sem_init(&semaphore_output, 0, 1);
    sem_init(&semaphore_file, 0, 1);

    pthread_create(&receiver, NULL, read_package, NULL);
    pthread_create(&output, NULL, output_package, NULL);
    

    sleep(10);
	
    free_queue_memory();
    free_names_memory();

	return 0;
}