#pragma once

#include "c_includes.h"

#define RECEIVER 0
#define FILEW 1
#define OUTPUT 2
#define GUI 3

void write_to_queue(char *data, int size, int id, parameters params);
int get_from_queue(char *data, int size, int id, parameters params);

void setup_queue(parameters params);
void free_queue();
void reset_queue();
void update_queue_index(int id, parameters params);

#ifndef NDEBUG
void set_writer_index(int v);
int get_writer_index();
void set_reader_index(int id, int v);
int get_reader_index(int id);
void set_queue(int i, char v);
char get_queue(int i);
#endif