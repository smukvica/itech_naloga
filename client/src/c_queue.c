#include <stdlib.h>
#include <semaphore.h>
#include <limits.h>
#include <string.h>

#include <stdio.h>

#include "c_queue.h"
#include "c_gui.h"

char *queue;
int writer_index = 0;
int reader_index[4] = {0, 0, 0, 0};

sem_t semaphore_q;

int index_is_smaller(int a_reader, int a_writer, int a_size, parameters a_params){
    if(a_reader < a_writer){
        if(a_reader + a_size <= a_writer)
            return 1;
    } else if(a_reader > a_writer){
        if(a_reader + a_size < (a_writer + a_params.queue_size))
            return 1;
    }

    return 0;
}


// writes a_data to queue
// only a_writer thread and receiver thread write (not at same time)
void write_to_queue(char *a_data, int a_size, int a_id, parameters a_params){
    int size_of_data = (a_params.number_of_fields * 
                        a_params.size_of_field + 
                        STATUS_FIELD_SIZE);
    // copy a_data to queue
    memcpy(queue + writer_index * size_of_data,
           a_data, 
           size_of_data * a_size);
    // semaphore to avoid multiple access to queue index
    sem_wait(&semaphore_q);
    writer_index += a_size;   // set a_writer index forward
    // keep it in range of max value
    writer_index %= a_params.queue_size;
    sem_post(&semaphore_q);
}

// gets a_data from queue
int get_from_queue(char *a_data, int a_size, int a_id, parameters a_params){
    // keep if we can get a_data from queue
    int can_write = 0;
    int r, w;

    sem_wait(&semaphore_q);
    // save indices
    r = reader_index[a_id];
    w = writer_index;
    sem_post(&semaphore_q);

    if(index_is_smaller(r, w, a_size, a_params))
        can_write = 1;

    // we can get a_data
    if(can_write){
        int size_of_data = (a_params.number_of_fields * 
                            a_params.size_of_field + 
                            STATUS_FIELD_SIZE);
        int i1, s1, i2, s2;
        // index of first chunk is current a_reader index
        i1 = r;
        // get a_size of first chunk
        // if index + a_size goes over we split the reading into two parts
        s1 = (a_params.queue_size - a_size >= r) ? a_size : a_params.queue_size - r;
        // index of second chunk is beggining of queue - 0
        i2 = 0;
        // a_size of second chunk
        // difference between requested a_size and first chunk a_size
        s2 = a_size - s1;

        // copy a_data
        memcpy(a_data, queue + i1 * size_of_data, size_of_data * s1);
        if(s2 != 0) // only copy if there is something to copy
            memcpy(a_data + size_of_data * s1, 
                   queue, 
                   size_of_data * s2);
        // update index
        reader_index[a_id] += a_size;
        // keep index in range
        reader_index[a_id] %= a_params.queue_size;
        // a_data copy successful return 0
        return 0;
    }
    // returns 1 if a_data copy wasn't possible
    return 1;
}

void setup_queue(parameters a_params){
    queue = (char*)malloc((sizeof(char) * 
                    a_params.number_of_fields * 
                    a_params.size_of_field + STATUS_FIELD_SIZE) * 
                    a_params.queue_size);

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
void update_queue_index(int a_id, parameters a_params){
    sem_wait(&semaphore_q);
    reader_index[a_id] = writer_index - c_samples;
    if(reader_index[a_id] < 0)
        reader_index[a_id] += a_params.queue_size;
    sem_post(&semaphore_q);
}

#ifndef NDEBUG
void set_writer_index(int a_v){
    writer_index = a_v;
}

int get_writer_index(){
    return writer_index;
}

void set_reader_index(int a_id, int a_v){
    reader_index[a_id] = a_v;
}

int get_reader_index(int a_id){
    return reader_index[a_id];
}

void set_queue(int a_i, char a_v){
    queue[a_i] = a_v;
}

char get_queue(int a_i){
    return queue[a_i];
}
#endif