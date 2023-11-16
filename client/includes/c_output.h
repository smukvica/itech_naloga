#pragma once

#include "c_includes.h"

void check_package_order(parameters params, int bpm_id, int package_id);
void *output_package(void *args);
void reset_package_order();

#ifndef NDEBUG
void set_check_bpm(int v);
void set_check_packet_num(int v);
void set_packet_errors(int v);
int get_check_bpm();
int get_check_packet_num();
int get_packet_errors();
#endif