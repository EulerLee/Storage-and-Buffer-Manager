#ifndef ENVIRONMENT
#define ENVIRONMENT

#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <assert.h>

#define FRAMESIZE 4096
#define DEFBUFSIZE 1024
#define MAXPAGES 50000

struct bFrame   // Frame in buffer
{
    char field[FRAMESIZE];
};

#endif
