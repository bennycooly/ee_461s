//
// Created by bennycooly on 9/11/16.
//

#include <stdio.h>

#include "pgroup.h"
#include "process.h"

void pgroup_init(pgroup* pg) {
    pg->pgid = 0;
    pg->state = '\0';
    pg->name = NULL;
    pg->size = 0;
    pg->capacity = MIN_PGROUP_CAPACITY;
    pg->processes = malloc(sizeof(process*) * pg->capacity);
}

void pgroup_exec(pgroup* pg) {
    for (uint32_t i = 0; i < pg->size; ++i) {
        if (i == 0) {
            process_exec(pg->processes[i], pg, true);
        }
        else {
            process_exec(pg->processes[i], pg, false);
        }
    }
}

void pgroup_insert(pgroup* pg, process* proc) {
    // check if we need to reallocate
    if (pg->size == pg->capacity) {   // double the capacity and reallocate
        pg->capacity *= 2;
        process** temp = realloc(pg->processes, sizeof(process*) * pg->capacity);
        if (!temp) {
            printf("Memory allocation failure.\n");
            exit(1);
        }
        pg->processes = temp;
    }
    pg->processes[pg->size] = proc;
    ++(pg->size);
}

void pgroup_destroy(pgroup* pg) {
    if (pg->size > 0) {
        for (uint32_t i = 0; i < pg->size; ++i) {
            free(pg->processes[i]);
        }
        free(pg->processes);
        pg->processes = NULL;
    }
    if (pg->name) {
        free(pg->name);
        pg->name = NULL;
    }
    free(pg);
}
