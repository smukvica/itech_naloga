#pragma once
#include "c_includes.h"

void load_params(const char *a_file, parameters *a_params);

void *file_writer(void *a_args);
void file_reader(const char *a_file, parameters *a_params);