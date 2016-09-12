//
// Created by bennycooly on 9/11/16.
//

#ifndef YASH_JOB_CONTROL_H
#define YASH_JOB_CONTROL_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct pgroup pgroup;

/**
 * Our session struct is basically a stack for process groups.
 *
 * We still need a pointer to the end of the stack so that
 */
typedef struct session {
    pid_t sid;
    pgroup* session_ptr;
    pgroup* pgroups[1];
    uint32_t pgroup_size;
    uint32_t pgroup_capacity;
} session;


/**
 * Functions.
 */
void set_session();

pgroup* session_get_active_pgroup(session*);

#endif //YASH_JOB_CONTROL_H
