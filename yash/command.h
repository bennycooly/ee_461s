//
// Created by bennycooly on 9/10/16.
//

#ifndef YASH_COMMAND_PARSER_H
#define YASH_COMMAND_PARSER_H

#include <stdint.h>



/**
 * Constants.
 */
// static const uint32_t   MAX_BUFFER_SIZE         = 256;
#define     MAX_BUFFER_SIZE             256
char        BUFFER[MAX_BUFFER_SIZE];


typedef struct Command {
    char* cmd;
    char* args[];
} Command;


/**
 * Functions.
 */
void cmd_read_line();
void cmd_execute();

#endif //YASH_COMMAND_PARSER_H
