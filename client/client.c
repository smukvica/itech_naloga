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

int queue_size = 100;
int number_of_packets = 1000;
int number_of_bpm = 1;
int file_entries = 100;
int num_of_fields = 3;
int size_of_field = 4;

char names[10][32];

sem_t semaphore_output;
sem_t semaphore_file;

package *queue;
int current_q_w = 0;
int current_q_r = 0;
int current_limit = 0;
int q_overflow = 0;

pthread_t receiver;
pthread_t output;
pthread_t writer;

char *file_queue[2];
int switch_buffer = 0;

int program_terminate = 0;

void setup_queue_memory(){
    queue = malloc(sizeof(package) * queue_size);
    for(int i = 0; i < queue_size; i++){
        queue[i].data = malloc(sizeof(char) * size_of_field * num_of_fields);
    }
}

void free_queue_memory(){
    for(int i = 0; i < queue_size; i++){
        free(queue[i].data);
    }
    free(queue);
}

void setup_file_memory(){
    file_queue[0] = malloc((sizeof(char) * size_of_field * num_of_fields + 4) * file_entries);

    file_queue[1] = malloc((sizeof(char) * size_of_field * num_of_fields + 4) * file_entries);
}

void free_file_memory(){
    free(file_queue[0]);
    free(file_queue[1]);
}

void read_file(const char *file){
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
}

void sigint_handler(){
    program_terminate = 1;
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
    signal(SIGINT, sigint_handler);

    if(strcmp(argv[1], "read_file") == 0){
        read_file(argv[2]);
        return 0;
    }

    arg_struct arguments;
    strcpy(arguments.ip, "127.0.0.1");
    arguments.port = 8888;

    int file_write = 1;
    int std_output = 1;

    int c = 1;
    if(strcmp(argv[1], "help") == 0){
        print_help();
        return 0;
    }
    while(c < argc - num_of_fields){
        if(strcmp(argv[c], "number_of_fields") == 0){
            num_of_fields = atoi(argv[c+1]);
            if(num_of_fields < 1 || num_of_fields > 10){
                printf("wrong usage of argument %s. see help\n", argv[c]);
                return 1;
            }
        }
        if(strcmp(argv[c], "size_of_field") == 0){
            size_of_field = atoi(argv[c+1]);
            if(size_of_field != 1 && size_of_field != 2 && size_of_field != 4){
                printf("wrong usage of argument %s. see help\n", argv[c]);
                return 1;
            }
        }
        if(strcmp(argv[c], "queue_size") == 0){
            queue_size = atoi(argv[c+1]);
        }
        if(strcmp(argv[c], "number_of_packets") == 0){
            number_of_packets = atoi(argv[c+1]);
        }
        if(strcmp(argv[c], "number_of_bpm") == 0){
            number_of_bpm = atoi(argv[c+1]);
            if(number_of_bpm < 1 || number_of_bpm > 4){
                printf("wrong usage of argument %s. see help\n", argv[c]);
                return 1;
            }
        }
        if(strcmp(argv[c], "file_entries") == 0){
            file_entries = atoi(argv[c+1]);
        }
        if(strcmp(argv[c], "ip") == 0){
            strcpy(arguments.ip, argv[c+1]);
        }
        if(strcmp(argv[c], "port") == 0){
            arguments.port = atoi(argv[c+1]);
        }
        if(strcmp(argv[c], "writer") == 0){
            file_write = atoi(argv[c+1]);
        }
        if(strcmp(argv[c], "output") == 0){
            std_output = atoi(argv[c+1]);
        }
        c += 2;
    }
    for(int k = 0; k < num_of_fields; k++){
        strcpy(names[k], argv[c+k]);
    }

    save_params();

    setup_queue_memory();
    setup_file_memory();

    sem_init(&semaphore_output, 0, 1);
    sem_init(&semaphore_file, 0, 1);

    pthread_create(&receiver, NULL, read_package, (void*)&arguments);
    if(std_output)
        pthread_create(&output, NULL, output_package, NULL);
    if(file_write)
        pthread_create(&writer, NULL, file_writer, NULL);
    

    while(program_terminate != 1){

    }

    
    pthread_join(receiver, NULL);
    if(std_output)
        pthread_join(output, NULL);
    if(file_write)
        pthread_join(writer, NULL);
	
    free_queue_memory();
    free_file_memory();

    sem_destroy(&semaphore_output);
    sem_destroy(&semaphore_file);

	return 0;
}



/*

compile:
gcc *.c -lpthread -fopenmp -o client

stop:
Ctrl + C


*/