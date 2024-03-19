#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>

#include "c_receiver.h"
#include "c_includes.h"
#include "c_queue.h"

// count pacages received
int received_packages = 0;

void print_exit_data(double time, int packages){
    printf("terminate receiver\n");
    sleep(1);
    printf("total packages:\t\t%d\n"
            "total time running:\t%f\n"
            "packages per second:\t%f\n", 
            packages, time, 
            (double)packages / time);
}

void *read_package(void *a_arguments){
    parameters *params = a_arguments;
    struct sockaddr_in server, my_addr;
    int socket_desc;
    fd_set rd;          // set to hold socket
    struct timeval tv;

    tv.tv_sec = 5;
    tv.tv_usec = 0;
    char server_reply[get_limit("number_of_fields", 1) * 
                      get_limit("size_of_field", 1)];
	
    int size_of_data = params->number_of_fields * 
                       params->size_of_field;

    // non-blocking socket
    socket_desc = socket(AF_INET , SOCK_DGRAM | SOCK_NONBLOCK, 0);

    
	if (socket_desc == -1)
	{
		printf("Could not create socket");
        exit(1);
	}

    // local settings to bind port to
	my_addr.sin_addr.s_addr = INADDR_ANY;
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(params->port);

    if (bind(socket_desc, (struct sockaddr*) &my_addr, sizeof(my_addr)) != 0)
    {
		perror("Failed to bind:");
		exit(1);
	}

    // measure time taken
    double t = omp_get_wtime();
    double stop_time = 0;
    double total_stop_time = 0;
    if(get_trace() == 1)
        printf("receiving data\n");
    
    socklen_t server_addr_len = sizeof(server);

    while(1){
        stop_time = omp_get_wtime();
        while(get_start_stop() == 0){
            sleep(0);
            if(get_program_terminate() == 1){
                close(socket_desc);
                double temp = omp_get_wtime();
                total_stop_time += temp - stop_time;
                t = temp - t - total_stop_time;
                print_exit_data(t, received_packages);
                return 0;
            }
        }
        double temp = omp_get_wtime() - stop_time;
        // only add time if we actually stop - more than 0.01 seconds
        if(temp > 0.01)
            total_stop_time += temp;
        
        // try to receive a package
        // check if there is data to read - select()
        // if it is we read bytes until we reach desired size
        int ret = 0;
        FD_ZERO( & rd);
        FD_SET(socket_desc, & rd);
        select(socket_desc + 1, &rd, NULL, NULL, &tv);
        if (FD_ISSET(socket_desc, &rd)){
            int bytes_read = 0;
            while(bytes_read < size_of_data){
                ret = recv( socket_desc, 
                            &server_reply + bytes_read, 
                            size_of_data - bytes_read,
                            0);
                bytes_read += ret;
            }
        }
        if( ret < 0){
            puts("recv failed");
            return 0;
        }
        // received something
        if (ret != 0){
            received_packages++;
            write_to_queue(server_reply, 1, RECEIVER, *params);
        }
        // signal to close the client or received nothing (server close)
        if(get_program_terminate() == 1){
            close(socket_desc);
            t = omp_get_wtime() - t - total_stop_time;
            print_exit_data(t, received_packages);
            return 0;
        }
    }
}

int get_received_packages() {
    return received_packages;
}