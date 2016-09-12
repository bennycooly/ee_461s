//
// Created by bennycooly on 9/11/16.
//

#ifndef YASH_PROCESS_H
#define YASH_PROCESS_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


static const uint32_t   MIN_ARGS_CAPACITY       = 5;        // default # of args in process

typedef struct pgroup pgroup;

/**
 * Structs.
 */
typedef struct process {
    pid_t pid;
    pid_t ppid;
    pid_t pgid;
    char state;
    // pipes/file redirection
    bool pipe_in;
    bool pipe_out;
    bool redirect_in;
    bool redirect_out;
    bool redirect_err;
    char* redirect_in_filename;
    char* redirect_out_filename;
    char* redirect_err_filename;
    char** args;
    uint32_t arg_size;
    uint32_t arg_capacity;
} process;

void process_init(process* proc);
void process_print(process* proc);
void process_exec(process* proc, pgroup* pg, bool is_first);

void process_destroy(process* proc);

#endif //YASH_PROCESS_H
