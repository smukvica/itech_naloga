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


// writes data to queue
// only writer thread and receiver thread write (not at same time)
void write_to_queue(char *data, int size, int id, parameters params){
    // copy data to queue
    memcpy(queue + writer_index * 
                (params.number_of_fields * params.size_of_field + 4),
           data, 
           (params.number_of_fields * params.size_of_field + 4) * size);
    // semaphore to avoid multiple access to queue index
    sem_wait(&semaphore_q);
    writer_index += size;   // set writer index forward
    // keep it in range of max value
    writer_index %= params.queue_size;
    sem_post(&semaphore_q);
}

// gets data from queue
int get_from_queue(char *data, int size, int id, parameters params){
    // keep if we can get data from queue
    int can_write = 0;

    sem_wait(&semaphore_q);
    // check if indices are valid
    if(reader_index[id] + size <= writer_index) can_write = 1;
    sem_post(&semaphore_q);

    // we can get data
    if(can_write){
        // copy data
        memcpy(data, 
               queue + (reader_index[id] % params.queue_size) * 
                       (params.number_of_fields * params.size_of_field + 4),
               (params.number_of_fields * params.size_of_field + 4) * size);
        // update index
        reader_index[id] += size;
        // keep index in range
        reader_index[id] %= params.queue_size;
        // data copy successful return 0
        return 0;
    }
    // returns 1 if data copy wasn't possible
    return 1;
}

void setup_queue(parameters params){
    queue = (char*)malloc((sizeof(char) * 
                    params.number_of_fields * 
                    params.size_of_field + 4) * 
                    params.queue_size);

    sem_init(&semaphore_q, 0, 1);
}

void free_queue(){
    free(queue);

    sem_destroy(&semaphore_q);
}

// resets to beginning when reading multiple files
void reset_queue(){
    writer_index = 0;
    reader_index[0] = 0;
    reader_index[1] = 0;
    reader_index[2] = 0;
    reader_index[3] = 0;
}

// updates the queue index to most recent entry - only GUI usage
void update_queue_index(int id){
    sem_wait(&semaphore_q);
    reader_index[id] = writer_index - 500;
    sem_post(&semaphore_q);
}