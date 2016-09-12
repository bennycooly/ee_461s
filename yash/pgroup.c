//
// Created by bennycooly on 9/11/16.
//

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

void pgroup_destroy(pgroup* pg) {
    if (pg->size > 0) {
        free(pg->processes);
        pg->processes = NULL;
    }
    if (pg->name) {
        free(pg->name);
        pg->name = NULL;
    }
    free(pg);
}
