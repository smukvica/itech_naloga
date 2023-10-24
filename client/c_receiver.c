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
    int socket_desc;
	struct sockaddr_in server;
	

    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
        exit(1);
	}

    parameters *args = arguments;

    char server_reply[args->num_of_fields * args->size_of_field + 4];

	server.sin_addr.s_addr = inet_addr(args->ip);
	server.sin_family = AF_INET;
	server.sin_port = htons(args->port);

	//Connect to remote server
	if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("connect error");
		exit(1);
	}
    
    int received_packages = 0;

    double t = omp_get_wtime();

    while(1){
        int ret = recv(socket_desc, &server_reply, args->num_of_fields * args->size_of_field + 4, 0);
        if( ret < 0){
            puts("recv failed");
            return 0;
        }
        if (ret == 0)
            program_terminate = 1;

        write_to_queue(server_reply, RECEIVER, *args);

        if(program_terminate == 1){
            t = (omp_get_wtime() - t);
            printf("terminate receiver\n");
            sleep(1);
            printf("total packages:\t\t%d\ntotal time running:\t%f\npackages per second:\t%f\n", received_packages, t, 
                                                                                                 (double)received_packages / t);
            return 0;
        }
        received_packages++;
    }
}