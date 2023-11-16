#pragma once
#include "c_includes.h"

void load_params(const char *file, parameters *params);

void *file_writer(void *args);
void file_reader(const char *file, parameters *params);