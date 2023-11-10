#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "c_includes.h"
#include "c_receiver.h"
#include "c_output.h"
#include "c_file.h"
#include "c_gui.h"
#include "c_queue.h"

pthread_t receiver;
pthread_t output;
pthread_t writer;
pthread_t gui;

int program_terminate = 0;
int setup_complete = 0;
int read_file = 0;
int start_stop = 0;
int pause_output = 0;

char filename[25];

// prints argument description using a set format
void print_argument(const char *arg, const char *explain, const char *usage){
    printf("%s\n\t%s\n\texample:\t%s\n", arg, explain, usage);
}

// prints all argument descriptions
void print_help(){
    printf("Help\n");
    printf("arguments:\n");
    print_argument("number_of_fields",
                   "set the number of data fields in packets. limited from 1 to 10",
                   "number_of_fields 5");
    print_argument("size_of_field",
                   "set the size of each data field in packet. limited to 1, 2, 4",
                   "size_of_field 4");
    print_argument("number_of_packets",
                   "set limit of packets sent.",
                   "number_of_packets 1000");
    print_argument("number_of_bpm",
                   "set the number of bpm cards simulated. limited to 1, 2, 3, 4",
                   "number_of_bpm 1");
    print_argument("queue_size",
                   "size of queue accepting new packets. limited from 200.000 to 100.000.000",
                   "queue_size 100.000");
    print_argument("file_entries",
                   "numbre of file entries in file when saving to file. limited from 500 to 10000",
                   "file_entries 500");
    print_argument("ip",
                   "set the ip of machine to connect to",
                   "ip 127.0.0.1");
    print_argument("port",
                   "set the port of the machine to connect to",
                   "port 8888");
    print_argument("writer",
                   "set to write to file or not",
                   "writer 1");
    print_argument("output",
                   "set to write to standard output or not",
                   "output 1");
    print_argument("read_file",
                   "read from a given file the data and print to standard out",
                   "read_file file1.bin");
    printf("at the end of all commands write the names of the data fields in order\n");
}

// reads arguments into parameters
int read_arguments(int argc, char *argv[], parameters *params){
    int c = 1;
    printf("%d\n", c);
    while(c < argc){
        
        if(strcmp(argv[c], "number_of_fields") == 0){
            params->number_of_fields = atoi(argv[c+1]);
            if(params->number_of_fields < 1 || params->number_of_fields > 10){
                printf("wrong usage of argument %s. see help\n", argv[c]);
                return 1;
            }
        }
        if(strcmp(argv[c], "size_of_field") == 0){
            params->size_of_field = atoi(argv[c+1]);
            if(params->size_of_field != 1 && params->size_of_field != 2 && params->size_of_field != 4){
                printf("wrong usage of argument %s. see help\n", argv[c]);
                return 1;
            }
        }
        if(strcmp(argv[c], "queue_size") == 0){
            params->queue_size = atoi(argv[c+1]);
            if(params->queue_size < 200000 || params->queue_size > 100000000){
                printf("wrong usage of argument %s. see help\n", argv[c]);
                return 1;
            }
        }
        if(strcmp(argv[c], "number_of_packets") == 0){
            params->number_of_packets = atoi(argv[c+1]);
        }
        if(strcmp(argv[c], "number_of_bpm") == 0){
            params->number_of_bpm = atoi(argv[c+1]);
            if(params->number_of_bpm < 1 || params->number_of_bpm > 4){
                printf("wrong usage of argument %s. see help\n", argv[c]);
                return 1;
            }
        }
        if(strcmp(argv[c], "file_entries") == 0){
            params->file_entries = atoi(argv[c+1]);
        }
        if(strcmp(argv[c], "ip") == 0){
            sscanf(argv[c+1], "%d.%d.%d.%d", &params->ip[0], &params->ip[1], &params->ip[2], &params->ip[3]);
        }
        if(strcmp(argv[c], "port") == 0){
            params->port = atoi(argv[c+1]);
        }
        if(strcmp(argv[c], "writer") == 0){
            params->file_write = atoi(argv[c+1]) == 1 ? true : false;
        }
        if(strcmp(argv[c], "output") == 0){
            params->std_output = atoi(argv[c+1]) == 1 ? true : false;
        }
        c += 2;
    }
    if(argc >= c + params->number_of_fields)
        for(int k = 0; k < params->number_of_fields; k++){
            if(strlen(argv[c+k]) >= 32)
                argv[c+k][32] = '\0';
            strcpy(params->names[k], argv[c+k]);
        }
    else
        printf("Not enough names parameters check again.\n");
}

void open_file_input(parameters params, char *filename){
    load_params(&params);
    
}

int main(int argc , char *argv[])
{
    // variable preventing from creating multiple receiver threads
    int receive = 0;
    // default setup for parameters
    parameters params = {.queue_size = 100,
                         .number_of_packets = 1000,
                         .number_of_bpm = 1,
                         .file_entries = 100,
                         .number_of_fields = 3,
                         .size_of_field = 4,
                         .names = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J"},
                         .file_write = true,
                         .std_output = true,
                         .port = 8888,
                         .ip = {127, 0, 0, 1}};

    //load_params(&params);
    // if argument is to read file 
    if(argc > 1){
        if(strcmp(argv[1], "read_file") == 0){
            strcpy(filename, argv[2]);
        } else if(strcmp(argv[1], "help") == 0){  // argument to print help
            print_help();
            return 0;
        }else if(read_arguments(argc, argv, &params) == 1) // reads other arguments
            return 1;
    }


    pthread_create(&gui, NULL, gui_setup, (void*)&params);

    // wait for gui to setup parameters
    while(setup_complete == 0){
        sleep(0);
        if(program_terminate == 1){
            pthread_join(gui, NULL);
            return 1;
        }
    }

    save_params(params);

    setup_queue(params);

    pthread_create(&receiver, NULL, read_package, (void*)&params);
    if(params.std_output)
        pthread_create(&output, NULL, output_package, (void*)&params);
    if(params.file_write)
        pthread_create(&writer, NULL, file_writer, (void*)&params);
    

    while(program_terminate != 1){
        if(read_file == 1){
            read_file = 0;
            pause_output = 1;
            reset_queue();
            reset_package_order();
            file_reader(&filename[0], params);
        }
        if(start_stop == 1 && receive == 0){
            reset_queue();
            reset_package_order();
            receive = 1;
        }
        if(start_stop == 0 && receive == 1){
            receive = 0;
        }
        sleep(0);
    }
    if(params.std_output)
        pthread_join(output, NULL);
    if(params.file_write)
        pthread_join(writer, NULL);
    pthread_join(gui, NULL);
    pthread_join(receiver, NULL);

    free_queue();

	return 0;
}

/*

compile:
gcc *.c -lpthread -fopenmp -lraylib -lm -o client

stop:
Ctrl + C


*/