//
// Created by bennycooly on 9/10/16.
//
#include "command.h"

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>



void cmd_execute() {
    char *myargs[4];
    myargs[0] = "ls";
    myargs[1] = "-all";
    myargs[2] = NULL;
    execvp(myargs[0], myargs);
    perror("Failed to exec");
}
