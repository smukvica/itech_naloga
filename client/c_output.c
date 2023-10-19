#include <stdio.h>
#include <semaphore.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>

#include "c_output.h"
#include "c_includes.h"

extern int queue_size;

extern int q_overflow;
extern int current_q_r;
extern int current_q_w;
extern int current_limit;

extern sem_t semaphore_output;

extern package *queue;

extern int number_of_bpm;
extern int num_of_fields;
extern int size_of_field;

int check_bpm = -1;
int check_packet_num = -1;



void *output_package(void *args){
    int i = 0;
    printf("sum\tx\ty\tstatus\n");
    while(i < 1000){
        while(q_overflow != 1 && current_q_r == current_limit){
            sem_wait(&semaphore_output);
            current_limit = current_q_w;
            sem_post(&semaphore_output);
            sleep(0);
        }
        char data[num_of_fields * size_of_field];
        int status;
        memcpy(data, queue[current_q_r % queue_size].data, sizeof(char) * num_of_fields * size_of_field);
        status = queue[current_q_r % queue_size].status;

        current_q_r++;
        if(current_q_r == INT_MAX){
            current_q_r %= queue_size;
            q_overflow = 0;
        }
        
        int out = 0;
        for(int i = 0; i < num_of_fields * size_of_field; i += size_of_field){
            memcpy(&out, &data[i], sizeof(char) * size_of_field);
            printf("%d\t", out);
        }
        printf("%d\n", status);

        int packets_sent = status >> 16;
        int bpm_id =       (status & (0xF << 2)) >> 2;

        printf("%d\t%d\n", bpm_id, packets_sent);

        
        if(check_bpm == -1)
            check_bpm = bpm_id;
        else
            if(check_bpm != bpm_id)
                printf("zaporedje bpm napačno\n");
            check_bpm = (bpm_id + 1) %number_of_bpm;
        
        if(check_packet_num == -1)
            check_packet_num = packets_sent;
        else
            if(check_packet_num + 1 != packets_sent)
                printf("zaporedje paketov napačno\n");
            check_packet_num = packets_sent;
        i++;
    }
}