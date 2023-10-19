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
extern sem_t semaphore_file;
extern package *queue;

extern int number_of_packets;
extern int num_of_fields;
extern int size_of_field;

extern int q_overflow;
extern int current_q_w;

void *read_package(void *arguments){
    int socket_desc;
	struct sockaddr_in server;
	char server_reply[num_of_fields * size_of_field + 4];

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

    char data[num_of_fields * size_of_field];
    int status;
    int i = 0;

    while(i < number_of_packets){
        if( recv(socket_desc, &server_reply, num_of_fields * size_of_field + 4, 0) < 0)
        {
            puts("recv failed");
        }
        memcpy(&data,server_reply,sizeof(char) * num_of_fields * size_of_field);
        memcpy(&status,server_reply + num_of_fields * size_of_field,sizeof(int));

        queue[current_q_w % queue_size].status = status;
        memcpy(queue[current_q_w % queue_size].data, &data, sizeof(char) * num_of_fields * size_of_field);
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