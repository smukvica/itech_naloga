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

// prints argument description using a set format
void print_argument(const char *a_arg, const char *a_explain, const char *a_usage, 
                    int a_limit_l, int a_limit_u){
    printf("%s\n", a_arg);
    printf("\t%s", a_explain);
    if(a_limit_l != -1 && a_limit_u != -1)
        printf(" limited from %d to %d.", a_limit_l, a_limit_u);

    printf("\n\texample:\t%s\n", a_usage);
}

// prints all argument descriptions
void print_help(){
    printf("Help\n");
    printf("arguments:\n");
    
    print_argument("number_of_fields",
                   "set the number of data fields in packets.",
                   "number_of_fields 5",
                    get_limit("number_of_fields", 0),
                    get_limit("number_of_fields", 1));
    print_argument("size_of_field",
                   "set the size of each data field in packet.",
                   "size_of_field 4",
                   get_limit("size_of_field", 0),
                   get_limit("size_of_field", 1));
    print_argument("number_of_packets",
                   "set limit of packets sent.",
                   "number_of_packets 1000", -1, -1);
    print_argument("number_of_bpm",
                   "set the number of bpm cards simulated.",
                   "number_of_bpm 1",
                   get_limit("number_of_bpm", 0),
                   get_limit("number_of_bpm", 1));
    print_argument("queue_size",
                   "size of queue accepting new packets.",
                   "queue_size 100.000",
                   get_limit("queue_size", 0),
                   get_limit("queue_size", 1));
    print_argument("file_entries",
                   "numbre of file entries in file when saving to file.",
                   "file_entries 500",
                   get_limit("file_entries", 0),
                   get_limit("file_entries", 1));
    print_argument("ip",
                   "set the ip of machine to connect to",
                   "ip 127.0.0.1", -1, -1);
    print_argument("port",
                   "set the port of the machine to connect to",
                   "port 8888", -1, -1);
    print_argument("writer",
                   "set to write to file or not",
                   "writer false", -1, -1);
    print_argument("output",
                   "set to write to standard output or not",
                   "output true", -1, -1);
    print_argument("read_file",
                   "read from a given file the data and print to standard out",
                   "read_file file1.bin", -1, -1);
    printf("at the end of all commands"
           " write the names of the data fields in order\n");
}

// reads arguments into parameters
int read_arguments(int a_argc, char *a_argv[], parameters *a_params){
    int c = 1;
    while(c < a_argc){
        if(strcmp(a_argv[c], "number_of_fields") == 0){
            a_params->number_of_fields = atoi(a_argv[c+1]);
            if(check_parameter_limits(a_argv[c], a_params->number_of_fields)){
                printf("wrong usage of argument %s. see help\n", a_argv[c]);
                return 1;
            }
        }
        if(strcmp(a_argv[c], "size_of_field") == 0){
            a_params->size_of_field = atoi(a_argv[c+1]);
            if(check_parameter_limits(a_argv[c], a_params->size_of_field)){
                printf("wrong usage of argument %s. see help\n", a_argv[c]);
                return 1;
            }
        }
        if(strcmp(a_argv[c], "queue_size") == 0){
            a_params->queue_size = atoi(a_argv[c+1]);
            if(check_parameter_limits(a_argv[c], a_params->queue_size)){
                printf("wrong usage of argument %s. see help\n", a_argv[c]);
                return 1;
            }
        }
        if(strcmp(a_argv[c], "number_of_packets") == 0){
            a_params->number_of_packets = atoi(a_argv[c+1]);
        }
        if(strcmp(a_argv[c], "number_of_bpm") == 0){
            a_params->number_of_bpm = atoi(a_argv[c+1]);
            if(check_parameter_limits(a_argv[c], a_params->number_of_bpm)){
                printf("wrong usage of argument %s. see help\n", a_argv[c]);
                return 1;
            }
        }
        if(strcmp(a_argv[c], "file_entries") == 0){
            a_params->file_entries = atoi(a_argv[c+1]);
            if(check_parameter_limits(a_argv[c], a_params->file_entries)){
                printf("wrong usage of argument %s. see help\n", a_argv[c]);
                return 1;
            }
        }
        if(strcmp(a_argv[c], "ip") == 0){
            sscanf(a_argv[c+1], "%d.%d.%d.%d", &a_params->ip[0], 
                                               &a_params->ip[1], 
                                               &a_params->ip[2], 
                                               &a_params->ip[3]);
        }
        if(strcmp(a_argv[c], "port") == 0){
            a_params->port = atoi(a_argv[c+1]);
        }
        if(strcmp(a_argv[c], "writer") == 0){
            if(strcmp(a_argv[c+1], "true") == 0)
                a_params->file_write = true;
            else if(strcmp(a_argv[c+1], "false") == 0)
                a_params->file_write = false;
            else
                printf("wrong usage of argument %s. see help\n", a_argv[c]);
        }
        if(strcmp(a_argv[c], "output") == 0){
            if(strcmp(a_argv[c+1], "true") == 0)
                a_params->std_output = true;
            else if(strcmp(a_argv[c+1], "false") == 0)
                a_params->std_output = false;
            else
                printf("wrong usage of argument %s. see help\n", a_argv[c]);
        }
        if(strcmp(a_argv[c], "folder") == 0){
            strcpy(a_params->save_folder, a_argv[c+1]);
        }
        if(strcmp(a_argv[c], "-trace") == 0){
            set_trace(1);
            c--;
        }
        if(strcmp(a_argv[c], "check_status") == 0){
            if(strcmp(a_argv[c+1], "true") == 0){
                a_params->check_status = true;
                a_params->status_field_size = 4;
            }
            else if(strcmp(a_argv[c+1], "false") == 0){
                a_params->check_status = false;
                a_params->status_field_size = 0;
            }
            else
                printf("wrong usage of argument %s. see help\n", a_argv[c]);
        }
        c += 2;
    }
    if(a_argc >= c + a_params->number_of_fields)
        for(int k = 0; k < a_params->number_of_fields; k++){
            if(strlen(a_argv[c+k]) >= 32)
                a_argv[c+k][31] = '\0';
            strcpy(a_params->names[k], a_argv[c+k]);
        }
}

int main(int argc , char *argv[])
{
    // variable preventing from creating multiple receiver threads
    int receive = 0;
    // default setup for parameters
    parameters params = {.queue_size = get_limit("queue_size", 0),
                         .number_of_packets = 1000,
                         .number_of_bpm = 1,
                         .file_entries = get_limit("file_entries", 0),
                         .number_of_fields = 3,
                         .size_of_field = 4,
                         .names = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J"},
                         .file_write = true,
                         .std_output = true,
                         .check_status = true,
                         .status_field_size = 4,
                         .port = 8888,
                         .ip = {127, 0, 0, 1}};

    load_params("config", &params);
    setup_includes();
    if(argc > 1){
        if(strcmp(argv[1], "read_file") == 0){
            strcpy(g_filename, argv[2]);
        } else if (strcmp(argv[1], "help") == 0){  // argument to print help
            print_help();
            return 0;
        } else if (read_arguments(argc, argv, &params) == 1) // reads other arguments
            return 1;
    }


    pthread_create(&gui, NULL, gui_setup, (void*)&params);

    // wait for gui to setup parameters
    while(get_setup_complete() == 0){
        sleep(0);
        if(get_program_terminate() == 1){
            pthread_join(gui, NULL);
            return 1;
        }
    }

    setup_queue(params);

    
    pthread_create(&output, NULL, output_package, (void*)&params);
    if(params.file_write)
        pthread_create(&writer, NULL, file_writer, (void*)&params);
    
    int created = 0;

    while(get_program_terminate() != 1){
        if(get_read_file() == 1){
            set_read_file(0);
            reset_queue();
            reset_package_order();
            file_reader(&g_filename[0], &params);
            clear_texture(params);
        }
        if(get_start_stop() == 1 && receive == 0){
            if(created == 0){
                pthread_create(&receiver, NULL, read_package, (void*)&params);
                created = 1;
            }
            reset_queue();
            reset_package_order();
            receive = 1;
        }
        if(get_start_stop() == 0 && receive == 1){
            receive = 0;
        }
        sleep(0);
    }
    pthread_join(output, NULL);
    if(params.file_write)
        pthread_join(writer, NULL);
    pthread_join(gui, NULL);
    if(created == 1)
        pthread_join(receiver, NULL);

    free_queue();
    free_includes();

    printf("packet error rate: \t%f\n", 
            (float)get_packet_errors()/get_received_packages());

	return 0;
}
