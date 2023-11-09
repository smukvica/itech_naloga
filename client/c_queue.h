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
void update_queue_index(int id);