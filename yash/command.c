//
// Created by bennycooly on 9/10/16.
//
#include "command.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>


void cmd_read_line() {

    if (fgets(BUFFER, MAX_BUFFER_SIZE, stdin) == NULL) {
        printf("Bye.\n");
        exit(0);
    }

    size_t len = strlen(BUFFER);            //
    BUFFER[len - 1] = '\0';                 // change the newline to string sentinel
    if (BUFFER[0] == '\04') {
        exit(0);
    }
}

void cmd_execute() {
    char *myargs[4];
    myargs[0] = "ls";
    myargs[1] = "-all";
    myargs[2] = NULL;
    execvp(myargs[0], myargs);
    perror("Failed to exec");
}
