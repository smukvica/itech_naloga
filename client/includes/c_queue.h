#pragma once

#include "c_includes.h"

#define RECEIVER 0
#define FILEW 1
#define OUTPUT 2
#define GUI 3

void write_to_queue(char *a_data, int a_size, int a_id, parameters a_params);
int get_from_queue(char *a_data, int a_size, int a_id, parameters a_params);

void setup_queue(parameters a_params);
void free_queue();
void reset_queue();
void update_queue_index(int a_id, parameters a_params);
int index_is_smaller(int a_reader, int a_writer, int a_size, parameters a_params);

#ifndef NDEBUG
void set_writer_index(int a_v);
int get_writer_index();
void set_reader_index(int a_id, int a_v);
int get_reader_index(int a_id);
void set_queue(int a_i, char a_v);
char get_queue(int a_i);
#endif