#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "c_includes.h"
#include "c_receiver.h"
#include "c_output.h"


int packet_size = 16;

int queue_size = 100;

sem_t semaphore_output;
sem_t semaphore_file;

package *queue;
int current_q_w = 0;
int current_q_r = 0;
int current_limit = 0;
int q_overflow = 0;

int number_of_packets = 1000;
int number_of_bpm = 1;

pthread_t receiver;
pthread_t output;
pthread_t file;

void get_queue_memory(){
    queue = malloc(sizeof(package) * queue_size);
}

void free_queue_memory(){
    free(queue);
}


int main(int argc , char *argv[])
{

    /*
        arguments:      name value
            queue_size
            number_of_packets
            number_of_bpm
    */
    int c = 1;
    while(c < argc){
        if(strcmp(argv[c], "queue_size") == 0){
            queue_size = atoi(argv[c+1]);
        }
        if(strcmp(argv[c], "number_of_packets") == 0){
            number_of_packets = atoi(argv[c+1]);
        }
        if(strcmp(argv[c], "number_of_bpm") == 0){
            number_of_bpm = atoi(argv[c+1]);
        }
        c += 2;
    }

    get_queue_memory();

    sem_init(&semaphore_output, 0, 1);
    sem_init(&semaphore_file, 0, 1);

    pthread_create(&receiver, NULL, read_package, NULL);
    pthread_create(&output, NULL, output_package, NULL);
    

    sleep(10);
	
    free_queue_memory();

	return 0;
}