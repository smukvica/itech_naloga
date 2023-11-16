#include <gtest/gtest.h>
#include "c_includes.c"
#include "c_queue.c"
#include "c_output.c"
#include "c_file.c"

int program_terminate = 0;
int trace = 0;

parameters params = {.queue_size = 200000,
                        .number_of_packets = 1000,
                        .number_of_bpm = 1,
                        .file_entries = 500,
                        .number_of_fields = 3,
                        .size_of_field = 4,
                        .names = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J"},
                        .file_write = true,
                        .std_output = true,
                        .port = 8888,
                        .ip = {127, 0, 0, 1}};

TEST(Queue, IndexUpdate) {

    extern int writer_index;
    extern int reader_index[4];
    writer_index = 543;
    reader_index[0] = 2;
    setup_queue(params);
    update_queue_index(0);
    free_queue();
    EXPECT_EQ(writer_index - 500, reader_index[0]);
}

TEST(Queue, WriteToQueue) {
    extern int writer_index;
    extern int reader_index[4];
    extern char *queue;
    char data[params.size_of_field * params.number_of_fields];
    for(int i = 0; i < params.size_of_field * params.number_of_fields; i++){
        data[i] = (char)3;
    }
    setup_queue(params);
    write_to_queue(&data[0], 1, RECEIVER, params);
    

    EXPECT_EQ(writer_index, 1);
    for(int i = 0; i < params.size_of_field * params.number_of_fields; i++){
        EXPECT_EQ(queue[i], data[i]);
    }

    free_queue();
}

TEST(Queue, WriteFromQueueOutput) {
    extern int writer_index;
    writer_index = 2;
    extern int reader_index[4];
    extern char *queue;
    char data[(params.size_of_field * params.number_of_fields + 4)];

    setup_queue(params);
    get_from_queue(&data[0], 1, OUTPUT, params);
    

    EXPECT_EQ(reader_index[OUTPUT], 1);
    for(int i = 0; i < params.size_of_field * params.number_of_fields + 4; i++){
        EXPECT_EQ(queue[i], data[i]);
    }
    free_queue();
}

TEST(Queue, WriteFromQueueGui) {
    extern int writer_index;
    writer_index = 501;
    extern int reader_index[4];
    extern char *queue;
    char data[(params.size_of_field * params.number_of_fields + 4) * 500];

    setup_queue(params);
    get_from_queue(&data[0], 500, GUI, params);
    

    EXPECT_EQ(reader_index[GUI], 500);
    for(int i = 0; i < (params.size_of_field * params.number_of_fields + 4) * 500; i++){
        EXPECT_EQ(queue[i], data[i]);
    }
    free_queue();
}

TEST(Output, PackageOrderCorrect) {
    extern int check_bpm;
    extern int check_pacet_num;
    check_bpm = -1;
    check_packet_num = -1;

    extern unsigned int packet_errors;
    packet_errors = 0;

    check_package_order(params, 0, 0);
    EXPECT_EQ(check_packet_num, 0);
    EXPECT_EQ(packet_errors, 0);
}

TEST(Output, PackageOrderIncorrect) {
    extern int check_bpm;
    extern int check_pacet_num;
    check_bpm = 0;
    check_packet_num = 0;

    extern unsigned int packet_errors;
    packet_errors = 0;

    check_package_order(params, 2, 4);

    EXPECT_NE(check_packet_num, 1);
    EXPECT_EQ(packet_errors, 1);
}

TEST(File, ReadingConfig) {
    load_params("../config", &params);
    
    EXPECT_EQ(params.number_of_fields, 5);
    EXPECT_EQ(params.size_of_field, 2);
    EXPECT_EQ(params.number_of_bpm, 1);
    EXPECT_EQ(params.queue_size, 200000);
    EXPECT_EQ(params.file_entries, 10000);
    EXPECT_EQ(params.ip[0], 127);
    EXPECT_EQ(params.ip[1], 0);
    EXPECT_EQ(params.ip[2], 0);
    EXPECT_EQ(params.ip[3], 1);
    EXPECT_EQ(params.port, 8888);
    EXPECT_EQ(params.std_output, true);
    EXPECT_EQ(params.file_write, false);
    EXPECT_STREQ(params.names[0], "a");
    EXPECT_STREQ(params.names[1], "b");
    EXPECT_STREQ(params.names[2], "c");
    EXPECT_STREQ(params.names[3], "d");
    EXPECT_STREQ(params.names[4], "e");
}

TEST(Limits, CheckParameter) {
    EXPECT_EQ(check_parameter_limits("queue_size", 500000), 0);
}

TEST(Limits, CheckParameterNonExistent) {
    EXPECT_EQ(check_parameter_limits("blabla", 0), 0);
}

TEST(Limits, GetLimit) {
    EXPECT_EQ(get_limit("file_entries", 0), 500);
}