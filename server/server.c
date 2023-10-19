#include <stdio.h>
#include <sys/socket.h> 
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int packets_sent = 0;
int number_of_packets = 1000;
int number_of_bpm = 1;
int current_bpm = 0;

int num_of_fields = 3;
int size_of_field = 4; // size in bytes

char **names;

void create_byte(int num, char *data){
    for(int i = 1; i < size_of_field; i++){
        data[i] = 0x0;
    }
    data[0] = num;
}

void create_packet(char *packet){
    char fields[num_of_fields * size_of_field];

    int status = 0;

    status |= packets_sent << 16;
    status |= (char)current_bpm << 2;

    current_bpm++;
    current_bpm %= number_of_bpm;

    char data[size_of_field];

    for(int i = 0; i < num_of_fields; i++){
        create_byte(i+1, data);
        memcpy(fields + i * size_of_field, &data, sizeof(char) * size_of_field);
    }
    memcpy(fields + num_of_fields * size_of_field, &status, sizeof(int));

    memcpy(packet, fields, num_of_fields*size_of_field + 4);

    int out = 0;
    for(int i = 0; i < num_of_fields * size_of_field; i += size_of_field){
        memcpy(&out, &fields[i], sizeof(char) * size_of_field);
        printf("%d\t", out);
    }
    printf("%d\n", status);
}

void setup_names_memory(int f){
    names = malloc(sizeof(char*) * f);
    for(int i = 0; i < f; i++){
        names[i] = malloc(sizeof(char) * 10);
    }
}

void free_names_memory(){
    for(int i = 0; i < num_of_fields; i++){
        free(names[i]);
    }
    free(names);
}

int main(int argc , char *argv[])
{

    /*
        arguments:      name value
            number_of_packets
            number_of_bpm
            structure #x# names - #x# (fields x size) names of fields then
    */
    int i = 1;
    while(i < argc){
        if(strcmp(argv[i], "structure") == 0){
            int f,s;
            sscanf(argv[i+1], "%dx%d", &f, &s);
            setup_names_memory(f);
            for(int k = 0; k < f; k++){
                strcpy(names[k], argv[i+2+k]);
            }
            i += f;
            num_of_fields = f;
            size_of_field = s;
        }
        if(strcmp(argv[i], "number_of_packets") == 0){
            number_of_packets = atoi(argv[i+1]);
        }
        if(strcmp(argv[i], "number_of_bpm") == 0){
            number_of_bpm = atoi(argv[i+1]);
        }
        i += 2;
    }

    srand(5);

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
	
	for(int i = 0; i < num_of_fields; i++){
        printf("%s\t", names[i]);
    }
    printf("status\n");

    char message[num_of_fields*size_of_field+4];
    while(1){

        create_packet(message);
        
        write(new_socket, message, num_of_fields*size_of_field+4);
        packets_sent++;
    }

    close(socket_desc);

    free_names_memory();

    return 0;
}