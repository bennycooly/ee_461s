//
// Created by bennycooly on 9/10/16.
//

#ifndef YASH_LINE_H
#define YASH_LINE_H

#include "session.h"

static const uint32_t   MAX_BUFFER_SIZE    = 2000;

// char        BUFFER[MAX_BUFFER_SIZE];



int line_read(char* buf, uint32_t max_buf_size);
int line_parse(char* buf, session* ses);
void line_exec(session* ses);

#endif //YASH_LINE_H
