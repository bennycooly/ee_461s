//
// Created by bennycooly on 9/11/16.
//

#include "job-control.h"

#include <unistd.h>

void set_session() {
    pid_t sid = setsid();

}
