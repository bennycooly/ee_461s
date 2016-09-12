//
// Created by bennycooly on 9/11/16.
//

#ifndef YASH_PGROUP_H
#define YASH_PGROUP_H

#include <stdlib.h>
#include <stdint.h>


static const uint32_t   MIN_PGROUP_CAPACITY     = 5;        // default # of processes in process group

typedef struct process process;                             // from process.h

typedef struct pgroup {
    pid_t pgid;
    char state;
    char* name;
    process** processes;
    uint32_t size;
    uint32_t capacity;
} pgroup;

void pgroup_exec(pgroup* pg);

void pgroup_free(pgroup* pg);

#endif //YASH_PGROUP_H
