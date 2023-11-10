#include <stdio.h>
#include <semaphore.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>

#include "c_output.h"
#include "c_includes.h"
#include "c_queue.h"

// save previous values of bpm and packet number initial is -1 (not set before)
int check_bpm = -1;
int check_packet_num = -1;

// count of errors
unsigned int bpm_errors = 0;
unsigned int packet_errors = 0;

// consecutive error count
int number_of_consecutive_errors = 0;
int previously_error = 0;

extern int program_terminate;

// checks if 2 consecutive packages are in order
void check_package_order(parameters params, int bpm_id, int package_id){
    if(check_bpm == -1) // no packages checked before no need to check if valid
        check_bpm = bpm_id;
    else
        if(check_bpm != bpm_id){    // expected value is different from actual
            printf("zaporedje bpm napačno\n");
            bpm_errors++;
        }
        check_bpm = (bpm_id + 1) % params.number_of_bpm; // update expected value for next package
    
    if(check_packet_num == -1) // no packages checked before no need to check if valid
        check_packet_num = package_id;
    else
        if(check_packet_num + 1 != package_id && check_packet_num != 65535){ // expected value is different from actual or number is larger than max available bits
            printf("zaporedje paketov napačno\n");
            if(previously_error == 0){  // first wrong package
                previously_error = 1;
            }
            if(previously_error == 1){  // multiple wrong packages
                number_of_consecutive_errors++;
            }
            packet_errors++;
        }
        else{   // reset consecutive errors
            previously_error = 0;
            number_of_consecutive_errors = 0;
        }
        check_packet_num = package_id;

    if(number_of_consecutive_errors > 100){ // if consecutive errors > 100 terminate
        program_terminate = 1;
    }
}

void *output_package(void *args){
    parameters params = *(parameters *)args;
    char data[params.number_of_fields * params.size_of_field + 4];
    for(int i = 0; i < params.number_of_fields; i++){
        printf("%s\t", params.names[i]);
    }
    printf("status\n");
    while(1){
        

        //  get data from queue
        int ret = get_from_queue(&data[0], 1, OUTPUT, params);
        if(ret != 1){   // value as retrieved
            unsigned int out = 0;
            int i;
            for(i = 0; i < params.number_of_fields * params.size_of_field; i += params.size_of_field){
                // print each field to stdout
                memcpy(&out, &data[i], sizeof(char) * params.size_of_field);    
                printf("%u\t", out);
            }
            // print also status field
            memcpy(&out, &data[i], sizeof(char) * 4);
            printf("%u\n", out);

            // extract package number and bpm values to check for errors
            unsigned int package_id = out >> 16;
            unsigned int bpm_id =     (out & (0xF << 2)) >> 2;
            
            check_package_order(params, bpm_id, package_id);
        }else{
            sleep(0);
        }
        // terminate program signal
        if(program_terminate == 1){
            printf("terminate output\n");
            sleep(1);
            printf("bpm errors:\t\t%d\npacket errors:\t\t%d\n", bpm_errors, packet_errors);
            return 0;
        }
    }
}

// resets checked packages values for file reading out of order
void reset_package_order(){
    check_bpm = -1;
    check_packet_num = -1;
}