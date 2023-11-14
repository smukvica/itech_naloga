#include <gtest/gtest.h>
#include "c_includes.h"
#include "c_queue.c"

TEST(Queue, IndexUpdate) {
    parameters params = {.queue_size = 100,
                         .number_of_packets = 1000,
                         .number_of_bpm = 1,
                         .file_entries = 100,
                         .number_of_fields = 3,
                         .size_of_field = 4,
                         .names = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J"},
                         .file_write = true,
                         .std_output = true,
                         .port = 8888,
                         .ip = {127, 0, 0, 1}};
    extern int writer_index;
    extern int reader_index[4];
    writer_index = 543;
    reader_index[0] = 2;
    setup_queue(params);
    update_queue_index(0);
    free_queue();
    EXPECT_EQ(writer_index - 500, reader_index[0]);
}