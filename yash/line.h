//
// Created by bennycooly on 9/10/16.
//

#ifndef YASH_LINE_H
#define YASH_LINE_H

#include "command.h"

static const uint32_t   MAX_BUFFER_SIZE         = 256;

// char        BUFFER[MAX_BUFFER_SIZE];



void line_read(char buf[], uint32_t max_buf_size);
void line_exec(int* pg_status);

#endif //YASH_LINE_H
