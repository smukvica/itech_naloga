#include <stdio.h>
#include <sys/socket.h> 
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define PACKET_SIZE 16

int packets_sent = 0;
int number_of_packets = 1;
int number_of_bpm = 1;
int current_bpm = 0;

int num_of_fields = 3;
int size_of_field = 4; // size in bytes

void create_packet(char *packet){
    int x = packets_sent;
    int y = number_of_packets - packets_sent;
    int sum = x + y;
    int status = 0;
    int data[3];
    data[0] = sum;
    data[1] = x;
    data[2] = y;

    char fields[num_of_fields * size_of_field];


    status |= packets_sent << 16;
    status |= (char)current_bpm << 2;

    current_bpm++;
    current_bpm %= number_of_bpm;

    for(int i = 0; i < num_of_fields; i++){
        memcpy(fields + i * size_of_field, &data[i], size_of_field);
    }
    memcpy(fields + num_of_fields * size_of_field, &status, sizeof(int));

    memcpy(packet, fields, num_of_fields*size_of_field + 4);

    printf("%d\t%d\t%d\t%d\n", sum, x, y, status);
}

int main(int argc , char *argv[])
{

    /*
        argumenti -
        1 število paketov
        2 različni BPM - število
    */

    number_of_packets = atoi(argv[1]);
    number_of_bpm = atoi(argv[2]);

    printf("%d\n", number_of_packets);

	int socket_desc, new_socket, c;
    struct sockaddr_in server, client;
        
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
        
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );
        
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("bind failed. Error");
        return -1;
    }
    printf("bind done\n");

    listen(socket_desc , 1);

    c = sizeof(struct sockaddr_in);
	new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
	if (new_socket<0)
	{
		printf("accept failed\n");
	}
	
	printf("Connection accepted\n");

    char message[PACKET_SIZE];
    while(packets_sent < number_of_packets){

        create_packet(message);
        
        write(new_socket, message, PACKET_SIZE);
        packets_sent++;
    }

    close(socket_desc);

    return 0;
}