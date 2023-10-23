#pragma once

#include <omp.h>

typedef struct package{
    char *data;
    unsigned int status;
}package;

typedef struct arg_struct{
    int port;
    char ip[15];
}arg_struct;