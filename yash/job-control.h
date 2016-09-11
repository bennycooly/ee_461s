//
// Created by bennycooly on 9/11/16.
//

#ifndef YASH_JOB_CONTROL_H
#define YASH_JOB_CONTROL_H

#include <stdlib.h>
#include <stdint.h>

/**
 * Structs.
 */
typedef struct process {
    pid_t pid;
    pid_t ppid;
    pid_t pgid;
    int status;
    char* cmd;
    char* args[];
} process;

typedef struct process_group {
    pid_t pgid;
    int status;
    uint32_t num_commands;
    process processes[];
} process_group;

typedef struct session {
    pid_t sid;
    process_group process_groups[];
} session;


/**
 * Functions.
 */
void set_session();

#endif //YASH_JOB_CONTROL_H
