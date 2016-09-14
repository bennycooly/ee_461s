//
// Created by bennycooly on 9/11/16.
//

#ifndef YASH_PGROUP_H
#define YASH_PGROUP_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


static const uint32_t   MIN_PGROUP_CAPACITY     = 5;        // default # of processes in process group

typedef struct session session;
typedef struct process process;                             // from process.h

typedef struct pgroup {
    pid_t pgid;
    char state;
    char* name;
    bool bg;
    process** processes;
    uint32_t size;
    uint32_t capacity;
} pgroup;

void pgroup_init(pgroup* pg);
void pgroup_exec(pgroup* pg, session* ses);
void pgroup_insert(pgroup* pg, process* proc);
void pgroup_destroy(pgroup* pg);

#endif //YASH_PGROUP_H
