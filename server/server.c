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

#define STATUS_FIELD_SIZE 4

const unsigned int c_limits_of_data[4] = {0xFF, 0xFFFF, 0xFFFFFF, 0xFFFFFFFF};
const int c_packet_num_start_bit = 16;
const int c_bmp_num_start_bit = 2;

const float c_pi_180 = 3.141/180;

int g_packets_sent = 0;
int g_number_of_packets = 100000;
int g_number_of_bpm = 1;
int g_current_bpm = 0;

int g_num_of_fields = 3;
int g_size_of_field = 4; // size in bytes

int g_send_rate = 10000;
int g_skip_rate = 0;


void create_byte(int a_num, char *a_data){
    unsigned int out = (unsigned int)(sin(a_num * c_pi_180) * 
                                      sin(a_num * c_pi_180) * 
                                      c_limits_of_data[g_size_of_field-1]);

    memcpy(a_data, &out, sizeof(char) * g_size_of_field);
}

// creates a data packet
void create_packet(char *a_packet){
    char fields[g_num_of_fields * g_size_of_field];

    unsigned int status = 0;

    // sets status bits
    status |= g_packets_sent << c_packet_num_start_bit;
    status |= g_current_bpm << c_bmp_num_start_bit;

    g_current_bpm++;
    g_current_bpm %= g_number_of_bpm;

    char data[g_size_of_field];

    // for each field create data and copies to fields
    for(int i = 0; i < g_num_of_fields; i++){
        create_byte(g_packets_sent, data);
        memcpy(fields + i * g_size_of_field, &data, sizeof(char) * g_size_of_field);
    }
    // copies status field at the end
    memcpy(fields + g_num_of_fields * g_size_of_field, &status, sizeof(int));

    // copies the final fields to packet to send out
    memcpy(a_packet, fields, g_num_of_fields * g_size_of_field + STATUS_FIELD_SIZE);

    // prints to std out
    unsigned int out = 0;
    for(int i = 0; i < g_num_of_fields * g_size_of_field; i += g_size_of_field){
        memcpy(&out, &fields[i], sizeof(char) * g_size_of_field);
        printf("%10u ", out);
    }
    printf("%u\n", status);
}

void print_argument(const char *a_arg, const char *a_explain, const char *a_usage){
    printf("%s\n\t%s\n\texample:\t%s\n", a_arg, a_explain, a_usage);
}

void print_help(){
    printf("Help\n");
    printf("arguments:\n");
    print_argument("number_of_fields",
                   "set the number of data fields in packets. limited from 1 to 10",
                   "number_of_fields 5");
    print_argument("size_of_field",
                   "set the size of each data field in packet. limited from 1 to 4",
                   "size_of_field 4");
    print_argument("number_of_packets",
                   "set limit of packets sent.",
                   "number_of_packets 1000");
    print_argument("number_of_bpm",
                   "set the number of bpm cards simulated. limited from 1 to 4",
                   "number_of_bpm 1");
    print_argument("send_rate",
                   "set the speed of transmission of packets",
                   "send_rate 10000");
}


int main(int argc , char *argv[])
{

    char ip[15] = "127.0.0.1";
    int port = 8888;
    srand(time(NULL));
    int i = 1;
    if(argc > 1)
        if(strcmp(argv[1], "help") == 0){
            print_help();
            return 0;
        }
    while(i < argc){
        if(strcmp(argv[i], "number_of_fields") == 0){
            g_num_of_fields = atoi(argv[i+1]);
            if(g_num_of_fields < 1 || g_num_of_fields > 10){
                printf("wrong usage of argument %s. see help\n", argv[i]);
                return 1;
            }
        }
        if(strcmp(argv[i], "size_of_field") == 0){
            g_size_of_field = atoi(argv[i+1]);
            if(g_size_of_field < 1 || g_size_of_field > 4){
                printf("wrong usage of argument %s. see help\n", argv[i]);
                return 1;
            }
        }
        if(strcmp(argv[i], "number_of_packets") == 0){
            g_number_of_packets = atoi(argv[i+1]);
        }
        if(strcmp(argv[i], "number_of_bpm") == 0){
            g_number_of_bpm = atoi(argv[i+1]);
            if(g_number_of_bpm < 1 || g_number_of_bpm > 4){
                printf("wrong usage of argument %s. see help\n", argv[i]);
                return 1;
            }
        }
        if(strcmp(argv[i], "ip") == 0){
            strcpy(ip, argv[i + 1]);
        }
        if(strcmp(argv[i], "port") == 0){
            port = atoi(argv[i+1]);
        }
        if(strcmp(argv[i], "send_rate") == 0){
            g_send_rate = atoi(argv[i+1]);
        }
        if(strcmp(argv[i], "skip_rate") == 0){
            g_skip_rate = atoi(argv[i+1]);
        }
        
        i += 2;
    }

	int socket_desc, new_socket, c;
    struct sockaddr_in server, client;
        
    //Create socket
    socket_desc = socket(AF_INET , SOCK_DGRAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
        
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_port = htons( port );

    connect(socket_desc, (struct sockaddr *)&server, sizeof(server));

    char message[g_num_of_fields * g_size_of_field + STATUS_FIELD_SIZE];

    double t = omp_get_wtime();

    // keeping frequency of packets per second
    double time_between_packets = 1.0 / g_send_rate;
    double current_time_between_packets = 0.0;
    double loop_time = 0.0;

    // sends out all packets
    while(g_packets_sent < g_number_of_packets){
        // checks time after last package sent
        loop_time = omp_get_wtime() - t;
        t += loop_time;
        current_time_between_packets += loop_time;
        // if time is more than frequency of packets send a new one
        if(current_time_between_packets >= time_between_packets){
            current_time_between_packets = 0.0;

            create_packet(message);
            if((rand() % 100) + 1 > g_skip_rate)
                send(socket_desc, message, g_num_of_fields * 
                                            g_size_of_field + 
                                            STATUS_FIELD_SIZE, 0);
            g_packets_sent++;
        }
    }

    close(socket_desc);

    return 0;
}
