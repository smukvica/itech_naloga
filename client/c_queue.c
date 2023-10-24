#include <stdlib.h>
#include <semaphore.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>

#include "c_queue.h"

char *file_queue2[2];
char *output_queue;
sem_t semaphore_1;
sem_t semaphore_2;

int output_q_index_receiver = 0;
int output_q_index_output = 0;
int output_q_overflow = 0;

void write_to_queue(char *data, int id, parameters params){
    printf("%d %d\n", params.num_of_fields, params.size_of_field);
    switch(id){
        case RECEIVER:
            memcpy(output_queue + (output_q_index_receiver % params.queue_size) * (params.num_of_fields * params.size_of_field + 4), 
                   data, params.num_of_fields * params.size_of_field + 4);
            sem_wait(&semaphore_1);
            output_q_index_receiver++;
            if(output_q_index_receiver == INT_MAX){
                output_q_index_receiver %= params.queue_size;
                output_q_overflow = 1;
            }
            sem_post(&semaphore_1);
            break;
        case OUTPUT:
            break;
        case FILE:
            break;
        case GUI:
            break;
        default:
            break;
    }
}

int get_from_queue(char *data, int id, parameters params){ // return 1 if not cool
    int can_write = 0;
    switch(id){
        case RECEIVER:
            break;
        case OUTPUT:
            sem_wait(&semaphore_1);
            if(output_q_index_output < output_q_index_receiver || output_q_overflow == 1) can_write = 1;
            sem_post(&semaphore_1);
            if(can_write){
                printf("here\n");
                memcpy(data, output_queue + (output_q_index_output % params.queue_size) * (params.num_of_fields * params.size_of_field + 4),
                       params.num_of_fields * params.size_of_field + 4);
                output_q_index_output++;
                if(output_q_index_output == INT_MAX){
                    output_q_index_output %= params.queue_size;
                    output_q_overflow = 0;
                }
                return 0;
            }
            else{
                return 1;
            }
            break;
        case FILE:
            break;
        case GUI:
            break;
        default:
            break;
    }
}

void setup_queue(parameters params){
    file_queue2[0] = malloc((sizeof(char) * params.num_of_fields * params.size_of_field + 4) * params.file_entries);
    file_queue2[1] = malloc((sizeof(char) * params.num_of_fields * params.size_of_field + 4) * params.file_entries);
    output_queue = malloc((sizeof(char) * params.num_of_fields * params.size_of_field + 4) * params.queue_size);

    sem_init(&semaphore_1, 0, 1);
    sem_init(&semaphore_2, 0, 1);
}

void free_queue(){
    free(file_queue2[0]);
    free(file_queue2[1]);
    free(output_queue);

    sem_destroy(&semaphore_1);
    sem_destroy(&semaphore_2);
}