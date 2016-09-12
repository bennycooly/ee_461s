//
// Created by bennycooly on 9/11/16.
//

#include "pgroup.h"
#include "process.h"

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

void pgroup_free(pgroup* pg) {
    if (pg->size > 0) {
        free(pg->processes);
        pg->processes = NULL;
    }
    free(pg);
}
