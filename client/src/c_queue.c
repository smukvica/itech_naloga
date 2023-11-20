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

int index_is_smaller(int reader, int writer, int size, parameters params){
    if(reader < writer){
        if(reader + size <= writer)
            return 1;
    } else if(reader > writer){
        if(reader + size < (writer + params.queue_size))
            return 1;
    }

    return 0;
}


// writes data to queue
// only writer thread and receiver thread write (not at same time)
void write_to_queue(char *data, int size, int id, parameters params){
    int size_of_data = (params.number_of_fields * params.size_of_field + 4);
    // copy data to queue
    memcpy(queue + writer_index * size_of_data,
           data, 
           size_of_data * size);
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
    int r, w;

    sem_wait(&semaphore_q);
    // save indices
    r = reader_index[id];
    w = writer_index;
    sem_post(&semaphore_q);

    if(index_is_smaller(r, w, size, params))
        can_write = 1;

    // we can get data
    if(can_write){
        int size_of_data = (params.number_of_fields * params.size_of_field + 4);
        int i1, s1, i2, s2;
        // index of first chunk is current reader index
        i1 = r;
        // get size of first chunk
        // if index + size goes over we split the reading into two parts
        s1 = (params.queue_size - size >= r) ? size : params.queue_size - r;
        // index of second chunk is beggining of queue - 0
        i2 = 0;
        // size of second chunk
        // difference between requested size and first chunk size
        s2 = size - s1;

        // copy data
        memcpy(data, queue + i1 * size_of_data, size_of_data * s1);
        if(s2 != 0) // only copy if there is something to copy
            memcpy(data + size_of_data * s1, 
                   queue, 
                   size_of_data * s2);
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
void update_queue_index(int id, parameters params){
    sem_wait(&semaphore_q);
    reader_index[id] = writer_index - 500;
    if(reader_index[id] < 0)
        reader_index[id] += params.queue_size;
    sem_post(&semaphore_q);
}

#ifndef NDEBUG
void set_writer_index(int v){
    writer_index = v;
}

int get_writer_index(){
    return writer_index;
}

void set_reader_index(int id, int v){
    reader_index[id] = v;
}

int get_reader_index(int id){
    return reader_index[id];
}

void set_queue(int i, char v){
    queue[i] = v;
}

char get_queue(int i){
    return queue[i];
}
#endif