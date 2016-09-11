//
// Created by bennycooly on 9/10/16.
//

#include "line.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

/**
 * Read a line from stdin into a buffer.
 */
void line_read(char buf[], uint32_t max_buf_size) {
    if (fgets(buf, max_buf_size, stdin) == NULL) {    // exit if EOF
        printf("Bye.\n");
        exit(0);
    }

    size_t len = strlen(buf);            //
    buf[len - 1] = '\0';                 // change the newline to string sentinel
    if (buf[0] == '\04') {
        exit(0);
    }
}


/**
 * Parse the buffer and dynamically allocate memory for the commands.
 */
void line_parse(char buf[]) {
    char* buf_ptr = buf;
    while(*buf_ptr != '\0') {   // process until EOF

        ++buf_ptr;
    }
}

void line_exec(int* pg_status) {

}
