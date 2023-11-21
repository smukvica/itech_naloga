#include <stdio.h>
#include <sys/socket.h> 
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <math.h>
#include <time.h>


int packets_sent = 0;
int number_of_packets = 100000;
int number_of_bpm = 1;
int current_bpm = 0;

int num_of_fields = 3;
int size_of_field = 4; // size in bytes

int send_rate = 10000;
int skip_rate = 0;

unsigned int limits_of_data[4] = {0xFF, 0xFFFF, 0xFFFFFF, 0xFFFFFFFF};

void create_byte(int num, char *data){
    unsigned int out = (unsigned int)(sin(num * 3.141/180.0) * sin(num * 3.141/180.0) * limits_of_data[size_of_field-1]);

    memcpy(data, &out, sizeof(char) * size_of_field);
}

// creates a data packet
void create_packet(char *packet){
    char fields[num_of_fields * size_of_field];

    unsigned int status = 0;

    // sets status bits
    status |= packets_sent << 16;
    status |= (char)current_bpm << 2;

    current_bpm++;
    current_bpm %= number_of_bpm;

    char data[size_of_field];

    // for each field create data and copies to fields
    for(int i = 0; i < num_of_fields; i++){
        create_byte(packets_sent, data);
        memcpy(fields + i * size_of_field, &data, sizeof(char) * size_of_field);
    }
    // copies status field at the end
    memcpy(fields + num_of_fields * size_of_field, &status, sizeof(int));

    // copies the final fields to packet to send out
    memcpy(packet, fields, num_of_fields*size_of_field + 4);

    // prints to std out
    unsigned int out = 0;
    for(int i = 0; i < num_of_fields * size_of_field; i += size_of_field){
        memcpy(&out, &fields[i], sizeof(char) * size_of_field);
        printf("%10u ", out);
    }
    printf("%u\n", status);
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
    print_argument("send_rate",
                   "set the speed of transmission of packets",
                   "send_rate 10000");
}


int main(int argc , char *argv[])
{
    srand(time(NULL));
    int i = 1;
    if(argc > 1)
        if(strcmp(argv[1], "help") == 0){
            print_help();
            return 0;
        }
    while(i < argc){
        if(strcmp(argv[i], "number_of_fields") == 0){
            num_of_fields = atoi(argv[i+1]);
            if(num_of_fields < 1 || num_of_fields > 10){
                printf("wrong usage of argument %s. see help\n", argv[i]);
                return 1;
            }
        }
        if(strcmp(argv[i], "size_of_field") == 0){
            size_of_field = atoi(argv[i+1]);
            if(size_of_field < 1 || size_of_field > 4){
                printf("wrong usage of argument %s. see help\n", argv[i]);
                return 1;
            }
        }
        if(strcmp(argv[i], "number_of_packets") == 0){
            number_of_packets = atoi(argv[i+1]);
        }
        if(strcmp(argv[i], "number_of_bpm") == 0){
            number_of_bpm = atoi(argv[i+1]);
            if(number_of_bpm < 1 || number_of_bpm > 4){
                printf("wrong usage of argument %s. see help\n", argv[i]);
                return 1;
            }
        }
        if(strcmp(argv[i], "send_rate") == 0){
            send_rate = atoi(argv[i+1]);
        }
        if(strcmp(argv[i], "skip_rate") == 0){
            skip_rate = atoi(argv[i+1]);
        }
        
        i += 2;
    }

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

    char message[num_of_fields*size_of_field+4];

    double t = omp_get_wtime();

    // keeping frequency of packets per second
    double time_between_packets = 1.0 / send_rate;
    double current_time_between_packets = 0.0;
    double loop_time = 0.0;

    // sends out all packets
    while(packets_sent < number_of_packets){
        // checks time after last package sent
        loop_time = omp_get_wtime() - t;
        t += loop_time;
        current_time_between_packets += loop_time;
        // if time is more than frequency of packets send a new one
        if(current_time_between_packets >= time_between_packets){
            current_time_between_packets = 0.0;

            create_packet(message);
            if((rand() % 100) + 1 > skip_rate)
                write(new_socket, message, num_of_fields*size_of_field+4);
            packets_sent++;
        }
    }

    close(socket_desc);

    return 0;
}
