#include <stdio.h>
#include <semaphore.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>

#include "c_output.h"
#include "c_includes.h"
#include "c_queue.h"

extern int queue_size;

extern int q_overflow;
extern int current_q_r;
extern int current_q_w;
extern int current_limit;

extern sem_t semaphore_output;

extern package *queue;

extern int number_of_packets;
extern int number_of_bpm;
extern int num_of_fields;
extern int size_of_field;

int check_bpm = -1;
int check_packet_num = -1;

unsigned int bpm_errors = 0;
unsigned int packet_errors = 0;

int number_of_consecutive_errors = 0;
int previously_error = 0;

extern char names[10][32];

extern int program_terminate;

void output_from_file(int number_of_elements){
    for(int i = 0; i < num_of_fields; i++){
        printf("%s\t", names[i]);
    }
    printf("status\n");
    int i = 0;
    while(i < number_of_elements){
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

void *output_package(void *args){
    parameters params = *(parameters *)args;
    for(int i = 0; i < params.num_of_fields; i++){
        printf("%s\t", params.names[i]);
    }
    printf("status\n");
    while(1){
        char data[params.num_of_fields * params.size_of_field + 4];

        int ret = get_from_queue(&data[0], OUTPUT, params);
        if(ret != 1){
            unsigned int out = 0;
            int i;
            for(i = 0; i < params.num_of_fields * params.size_of_field; i += params.size_of_field){
                memcpy(&out, &data[i], sizeof(char) * params.size_of_field);
                printf("%d\t", out);
            }
            memcpy(&out, &data[i], sizeof(char) * 4);
            printf("%u\n", out);

            unsigned int packets_sent = out >> 16;
            unsigned int bpm_id =       (out & (0xF << 2)) >> 2;
            
            if(check_bpm == -1)
                check_bpm = bpm_id;
            else
                if(check_bpm != bpm_id){
                    printf("zaporedje bpm napačno\n");
                    bpm_errors++;
                }
                check_bpm = (bpm_id + 1) %number_of_bpm;
            
            if(check_packet_num == -1)
                check_packet_num = packets_sent;
            else
                if(check_packet_num + 1 != packets_sent && check_packet_num != 65535){
                    printf("zaporedje paketov napačno\n");
                    if(previously_error == 0){
                        previously_error = 1;
                    }
                    if(previously_error == 1){
                        number_of_consecutive_errors++;
                    }
                    packet_errors++;
                }
                else{
                    previously_error = 0;
                    number_of_consecutive_errors = 0;
                }
                check_packet_num = packets_sent;
        }
        if(program_terminate == 1){
            printf("terminate output\n");
            sleep(1);
            printf("bpm errors:\t\t%d\npacket errors:\t\t%d\n", bpm_errors, packet_errors);
            return 0;
        }

        if(number_of_consecutive_errors > 100){
            program_terminate = 1;
        }
    }
}