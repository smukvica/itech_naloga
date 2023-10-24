#pragma once

#include "c_includes.h"

#define RECEIVER 0
#define OUTPUT 1
#define FILEW 2
#define GUI 3

void write_to_queue(char *data, int id, parameters params);
int get_from_queue(char *data, int id, parameters params);

void setup_queue(parameters params);
void free_queue();