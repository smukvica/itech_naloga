#pragma once
#include "c_includes.h"

void save_params(parameters params);
void load_params(parameters *params);

void *file_writer(void *args);
void file_reader(const char *file, parameters *params);