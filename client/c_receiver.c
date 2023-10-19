#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <limits.h>
#include <stdlib.h>

#include "c_receiver.h"
#include "c_includes.h"

extern int queue_size;
extern sem_t semaphore_output;
extern package queue[];

extern int number_of_packets;
extern int packet_size;

extern int q_overflow;
extern int current_q_w;

void *read_package(void *arguments){
    int socket_desc;
	struct sockaddr_in server;
	char server_reply[packet_size];

    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
        exit(1);
	}
		
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
	server.sin_port = htons(8888);

	//Connect to remote server
	if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("connect error");
		exit(1);
	}
	
	puts("Connected\n");

    int sum, x, y, status;
    int i = 0;

    while(i < number_of_packets){
        if( recv(socket_desc, &server_reply, packet_size, 0) < 0)
        {
            puts("recv failed");
        }
        memcpy(&sum,server_reply,sizeof(int));
        memcpy(&x,server_reply+4,sizeof(int));
        memcpy(&y,server_reply+8,sizeof(int));
        memcpy(&status,server_reply+12,sizeof(int));


        queue[current_q_w % queue_size] = (package){.sum = sum, .x = x, .y = y, .status = status};
        sem_wait(&semaphore_output);
        current_q_w++;
        if(current_q_w == INT_MAX){
            current_q_w %= queue_size;
            q_overflow = 1;
        }
        sem_post(&semaphore_output);
        i++;
    }
}