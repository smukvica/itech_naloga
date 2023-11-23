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

#define MY_PORT 2048


// count pacages received
int received_packages = 0;

void *read_package(void *a_arguments){
    parameters *params = a_arguments;
    struct sockaddr_in server, my_addr;
    int socket_desc;
    char server_reply[get_limit("number_of_fields", 1) * 
                      get_limit("size_of_field", 1) + params->status_field_size];
	
    socket_desc = socket(AF_INET , SOCK_DGRAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
        exit(1);
	}

    // server settings
    char ip[15];
    sprintf(ip, "%d.%d.%d.%d", params->ip[0], 
                               params->ip[1], 
                               params->ip[2], 
                               params->ip[3]);

    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(params->port);

    // local settings to bind port to
	my_addr.sin_addr.s_addr = INADDR_ANY;
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MY_PORT);

    if (bind(socket_desc, (struct sockaddr*) &my_addr, sizeof(my_addr)) != 0)
    {
		perror("Failed to bind:");
		exit(1);
	}

    // measure time taken
    double t = omp_get_wtime();
    if(get_trace() == 1)
        printf("receiving data\n");
    
    socklen_t server_addr_len = sizeof(server);

    while(1){
        while(get_start_stop() == 0){
            sleep(0);
            if(get_program_terminate() == 1)
                return 0;
        }
        // try to receive a package
        int ret = recvfrom(socket_desc, 
                           &server_reply, 
                           params->number_of_fields * 
                            params->size_of_field + 
                            params->status_field_size,
                           0,
                           (struct sockaddr*) &server,
                           &server_addr_len);
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
        if(get_program_terminate() == 1 || ret == 0){
            close(socket_desc);
            t = (omp_get_wtime() - t);
            printf("terminate receiver\n");
            sleep(1);
            printf("total packages:\t\t%d\n"
                   "total time running:\t%f\n"
                   "packages per second:\t%f\n", 
                    received_packages, t, 
                    (double)received_packages / t);
            return 0;
        }
    }
}

int get_received_packages() {
    return received_packages;
}