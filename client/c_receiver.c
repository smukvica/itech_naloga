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

extern int program_terminate;

void *read_package(void *arguments){
    parameters *params = arguments;
    int socket_desc;
	struct sockaddr_in server;
    char server_reply[params->number_of_fields * params->size_of_field + 4];
	
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
        exit(1);
	}

	server.sin_addr.s_addr = inet_addr(params->ip);
	server.sin_family = AF_INET;
	server.sin_port = htons(params->port);

	//Connect to remote server
	if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("connect error");
		exit(1);
	}
    
    // count pacages received
    int received_packages = 0;
    // measure time taken
    double t = omp_get_wtime();

    while(1){
        // try to receive a package
        int ret = recv(socket_desc, &server_reply, params->number_of_fields * params->size_of_field + 4, 0);
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
        if(program_terminate == 1 || ret == 0){
            t = (omp_get_wtime() - t);
            printf("terminate receiver\n");
            sleep(1);
            printf("total packages:\t\t%d\ntotal time running:\t%f\n
                    packages per second:\t%f\n", received_packages, t, 
                    (double)received_packages / t);
            return 0;
        }
    }
}