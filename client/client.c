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

void read_file(const char *file){
    /*
    FILE *write;
    write = fopen(file,"rb");  // r for read, b for binary

    load_params("params.txt");

    setup_queue_memory();

    char buffer[num_of_fields * size_of_field + 4];

    for(int i = 0; i < 100; i++){
        queue[i].data = malloc(num_of_fields * size_of_field);
        fread(buffer, sizeof(char) * num_of_fields * size_of_field + 4, 1,write);
        memcpy(queue[i].data, buffer, num_of_fields * size_of_field);
        memcpy(&queue[i].status, buffer + num_of_fields * size_of_field, 4);
    }

    output_from_file(file_entries);

    free_queue_memory();
    */
}

void print_argument(const char *arg, const char *explain, const char *usage){
    printf("%s\n\t%s\n\texample:\t%s\n", arg, explain, usage);
}

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
                   "size of queue accepting new packets",
                   "queue_size 100");
    print_argument("file_entries",
                   "numbre of file entries in file when saving to file",
                   "file_entries 100");
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

int main(int argc , char *argv[])
{
    parameters params = {.queue_size = 100,
                         .number_of_packets = 1000,
                         .number_of_bpm = 1,
                         .file_entries = 100,
                         .num_of_fields = 3,
                         .size_of_field = 4,
                         .names = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J"},
                         .file_write = 1,
                         .std_output = 1,
                         .port = 8888,
                         .ip = "127.0.0.1"};

    if(strcmp(argv[1], "read_file") == 0){
        read_file(argv[2]);
        return 0;
    }

    int c = 1;
    if(strcmp(argv[1], "help") == 0){
        print_help();
        return 0;
    }
    
    while(c < argc - params.num_of_fields){
        if(strcmp(argv[c], "number_of_fields") == 0){
            params.num_of_fields = atoi(argv[c+1]);
            if(params.num_of_fields < 1 || params.num_of_fields > 10){
                printf("wrong usage of argument %s. see help\n", argv[c]);
                return 1;
            }
        }
        if(strcmp(argv[c], "size_of_field") == 0){
            params.size_of_field = atoi(argv[c+1]);
            if(params.size_of_field != 1 && params.size_of_field != 2 && params.size_of_field != 4){
                printf("wrong usage of argument %s. see help\n", argv[c]);
                return 1;
            }
        }
        if(strcmp(argv[c], "queue_size") == 0){
            params.queue_size = atoi(argv[c+1]);
        }
        if(strcmp(argv[c], "number_of_packets") == 0){
            params.number_of_packets = atoi(argv[c+1]);
        }
        if(strcmp(argv[c], "number_of_bpm") == 0){
            params.number_of_bpm = atoi(argv[c+1]);
            if(params.number_of_bpm < 1 || params.number_of_bpm > 4){
                printf("wrong usage of argument %s. see help\n", argv[c]);
                return 1;
            }
        }
        if(strcmp(argv[c], "file_entries") == 0){
            params.file_entries = atoi(argv[c+1]);
        }
        if(strcmp(argv[c], "ip") == 0){
            strcpy(params.ip, argv[c+1]);
        }
        if(strcmp(argv[c], "port") == 0){
            params.port = atoi(argv[c+1]);
        }
        if(strcmp(argv[c], "writer") == 0){
            params.file_write = atoi(argv[c+1]);
        }
        if(strcmp(argv[c], "output") == 0){
            params.std_output = atoi(argv[c+1]);
        }
        c += 2;
    }
    for(int k = 0; k < params.num_of_fields; k++){
        strcpy(params.names[k], argv[c+k]);
    }
    

    int ret = gui_setup(&params);
    if(ret)
        return 1;
    save_params(params);

    setup_queue(params);

    pthread_create(&gui, NULL, gui_draw, NULL);

    pthread_create(&receiver, NULL, read_package, (void*)&params);
    if(params.std_output)
        pthread_create(&output, NULL, output_package, (void*)&params);
    if(params.file_write)
        pthread_create(&writer, NULL, file_writer, (void*)&params);
    

    while(program_terminate != 1){

    }

    
    pthread_join(receiver, NULL);
    if(params.std_output)
        pthread_join(output, NULL);
    if(params.file_write)
        pthread_join(writer, NULL);
    pthread_join(gui, NULL);

    free_queue();

	return 0;
}

/*

compile:
gcc *.c -lpthread -fopenmp -o client

stop:
Ctrl + C


*/