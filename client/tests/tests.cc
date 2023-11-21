#include <gtest/gtest.h>
#include <raylib.h>

extern "C" {
#include "c_includes.h"
#include "c_receiver.h"
#include "c_output.h"
#include "c_file.h"
#include "c_gui.h"
#include "c_queue.h"
}

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

TEST(Queue, CanWriteCheck){
    setup_queue(params);
    // both indexes are 0
    EXPECT_EQ(index_is_smaller(0, 0, 1, params), 0);

    // both indexes are 0, size bigger
    EXPECT_EQ(index_is_smaller(0, 0, 10000, params), 0);

    // reader is bigger, size 1 - overflow
    EXPECT_EQ(index_is_smaller(199950, 100, 1, params), 1);

    // reader is bigger, size bigger - overflow
    EXPECT_EQ(index_is_smaller(199950, 100, 10000, params), 0);

    // writer is bigger, size one
    EXPECT_EQ(index_is_smaller(100, 500, 1, params), 1);

    // writer is bigger, size bigger
    EXPECT_EQ(index_is_smaller(100, 500, 100, params), 1);

    // writer is bigger, size bigger
    EXPECT_EQ(index_is_smaller(100, 500, 1000, params), 0);
}

TEST(Queue, IndexUpdate) {
    set_writer_index(542);
    set_reader_index(0, 2);

    setup_queue(params);
    update_queue_index(0, params);
    free_queue();
    EXPECT_EQ(get_writer_index() - 500, get_reader_index(0));
}

TEST(Queue, IndexUpdateOverflow) {
    set_writer_index(250);
    set_reader_index(0, 199955);
    setup_queue(params);
    update_queue_index(0, params);
    free_queue();
    EXPECT_EQ(params.queue_size - (500 - get_writer_index()), get_reader_index(0));
}

TEST(Queue, WriteToQueue) {
    char data[params.size_of_field * params.number_of_fields];
    for(int i = 0; i < params.size_of_field * params.number_of_fields; i++){
        data[i] = (char)3;
    }
    setup_queue(params);
    write_to_queue(&data[0], 1, RECEIVER, params);
    

    EXPECT_EQ(get_writer_index(), 1);
    for(int i = 0; i < params.size_of_field * params.number_of_fields; i++){
        EXPECT_EQ(get_queue(i), data[i]);
    }

    free_queue();
}

TEST(Queue, WriteFromQueueOutput) {
    char data[(params.size_of_field * params.number_of_fields + 4)];

    set_writer_index(1);
    set_reader_index(OUTPUT, 0);

    setup_queue(params);
    for(int i = 0; i < params.size_of_field * params.number_of_fields + 4; i++){
        set_queue(i, i % 256);
    }
    get_from_queue(&data[0], 1, OUTPUT, params);

    EXPECT_EQ(get_reader_index(OUTPUT), 1);
    for(int i = 0; i < params.size_of_field * params.number_of_fields + 4; i++){
        EXPECT_EQ(get_queue(i), data[i]);
    }
    free_queue();
}

TEST(Queue, WriteFromQueueOutputOverflow) {
    char data[(params.size_of_field * params.number_of_fields + 4)] = {22};

    set_writer_index(100);
    set_reader_index(OUTPUT, 199950);

    setup_queue(params);
    for(int i = 0; i < params.queue_size * (params.size_of_field * params.number_of_fields + 4); i++){
        set_queue(i, i % 256);
    }
    for(int i = 0; i < 100;i++){
        get_from_queue(&data[0], 1, OUTPUT, params);
        
        for(int j = 0; j < (params.size_of_field * params.number_of_fields + 4); j++){
            EXPECT_EQ(get_queue(((199950 + i) % params.queue_size) * (params.size_of_field * params.number_of_fields + 4) + j), data[j]);
        }
    }
    free_queue();
}

TEST(Queue, WriteFromQueueGui) {
    set_writer_index(501);
    char data[(params.size_of_field * params.number_of_fields + 4) * 500];

    setup_queue(params);
    for(int i = 0; i < params.queue_size * (params.size_of_field * params.number_of_fields + 4); i++){
        set_queue(i, (char)i % 256);
    }
    get_from_queue(&data[0], 500, GUI, params);

    EXPECT_EQ(get_reader_index(GUI), 500);
    for(int i = 0; i < (params.size_of_field * params.number_of_fields + 4) * 500; i++){
        EXPECT_EQ(get_queue(i), data[i]);
    }
    free_queue();
}

TEST(Queue, WriteFromQueueGuiOverflow) {
    set_writer_index(300);
    set_reader_index(GUI, 199750);
    char data[(params.size_of_field * params.number_of_fields + 4) * 500] = { 0 };

    setup_queue(params);
    for(int i = 0; i < params.queue_size * (params.size_of_field * params.number_of_fields + 4); i++){
        set_queue(i, i % 256);
    }
    get_from_queue(&data[0], 500, GUI, params);
    
    int size_of_data = (params.number_of_fields * params.size_of_field + 4);

    EXPECT_EQ(get_reader_index(GUI), 250);
    for(int i = 0; i < 500; i++){
        EXPECT_EQ(get_queue(((199750 + i) % params.queue_size) * size_of_data), data[i * size_of_data]);
    }
    free_queue();
}

TEST(Queue, WriteFromQueueGuiMaxLimit) {
    set_writer_index(300);
    set_reader_index(GUI, 199500);
    char data[(params.size_of_field * params.number_of_fields + 4) * 500] = { 0 };

    setup_queue(params);
    for(int i = 0; i < params.queue_size * (params.size_of_field * params.number_of_fields + 4); i++){
        set_queue(i, i % 256);
    }
    get_from_queue(&data[0], 500, GUI, params);
    
    int size_of_data = (params.number_of_fields * params.size_of_field + 4);

    EXPECT_EQ(get_reader_index(GUI), 0);
    for(int i = 0; i < 500; i++){
        EXPECT_EQ(get_queue(((199500 + i) % params.queue_size) * size_of_data), data[i * size_of_data]);
    }
    free_queue();
}

TEST(Output, PackageOrderCorrect) {
    set_check_bpm(-1);
    set_check_packet_num(-1);

    set_packet_errors(0);

    check_package_order(params, 0, 0);
    EXPECT_EQ(get_check_packet_num(), 0);
    EXPECT_EQ(get_packet_errors(), 0);
}

TEST(Output, PackageOrderIncorrect) {
    set_check_bpm(0);
    set_check_packet_num(0);

    set_packet_errors(0);

    check_package_order(params, 2, 4);

    EXPECT_NE(get_check_packet_num(), 1);
    EXPECT_EQ(get_packet_errors(), 1);
}

TEST(File, ReadingConfig) {
    load_params("../../config", &params);
    
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

TEST(Gui, ClearTexture) {
    setup_includes();

    create_texture(params);
    clear_texture(params);

    for(int i = 0; i < c_texture_size; i++){
        for(int j = 0; j < c_screen_size; j++){
            if(j%(c_screen_size / params.number_of_fields) == 0 && j != 0)
                EXPECT_EQ(g_texture_data[j * c_texture_size + i], 127);
            else
                EXPECT_EQ(g_texture_data[j * c_texture_size + i], 255);
        }
    }

    delete_texture();
    free_includes();
}

TEST(Gui, UpdateTextureData) {
    setup_includes();
    create_texture(params);
    clear_texture(params);

    char data[(get_limit("number_of_fields", 1) * 
            get_limit("size_of_field", 1) + 4) * c_samples]; 

    create_image_from_data(&data[0], params);

    for(int i = 0; i < c_texture_size; i++){
        for(int j = 0; j < c_screen_size; j++){
            if(j%(c_screen_size / params.number_of_fields) == 0 && j != 0)
                EXPECT_EQ(g_texture_data[j * c_texture_size + i], 0);
            else
                EXPECT_EQ(g_texture_data[j * c_texture_size + i], 255);
        }
    }

    delete_texture();
    free_includes();
}