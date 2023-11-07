#include <stdlib.h>
#include <semaphore.h>
#include <limits.h>
#include <string.h>

#include <stdio.h>

#include "c_queue.h"

char *queue;
int writer_index = 0;
int reader_index[4] = {0, 0, 0, 0};

sem_t semaphore_q;

void write_to_queue(char *data, int size, int id, parameters params){
    memcpy(queue + writer_index % params.queue_size * (params.num_of_fields * params.size_of_field + 4),
            data, (params.num_of_fields * params.size_of_field + 4) * size);
    sem_wait(&semaphore_q);
    writer_index += size;
    if(writer_index >= INT_MAX)
    writer_index %= params.queue_size;
    sem_post(&semaphore_q);
}

int get_from_queue(char *data, int size, int id, parameters params){ // return 1 if not cool
    
    int can_write = 0;

    
    if(id == GUI){
        sem_wait(&semaphore_q);
        reader_index[id] = writer_index - size;
        sem_post(&semaphore_q);
        if(reader_index[id] >= 0) can_write = 1;
    }else{
        sem_wait(&semaphore_q);
        if(reader_index[id] + size <= writer_index) can_write = 1;
        sem_post(&semaphore_q);
    }

    if(can_write){
        memcpy(data, queue + (reader_index[id] % params.queue_size) * (params.num_of_fields * params.size_of_field + 4),
               (params.num_of_fields * params.size_of_field + 4) * size);
        reader_index[id] += size;
        if(reader_index[id] >= INT_MAX) reader_index[id] %= params.queue_size;
        return 0;
    }
    return 1;
}

void setup_queue(parameters params){
    queue = malloc((sizeof(char) * params.num_of_fields * params.size_of_field + 4) * params.queue_size);

    sem_init(&semaphore_q, 0, 1);
}

void free_queue(){
    free(queue);

    sem_destroy(&semaphore_q);
}

void reset_queue(){
    writer_index = 0;
    reader_index[0] = 0;
    reader_index[1] = 0;
    reader_index[2] = 0;
    reader_index[3] = 0;
}