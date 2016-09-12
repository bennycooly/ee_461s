//
// Created by bennycooly on 9/11/16.
//

#include "session.h"

void set_session() {

}

pgroup* session_get_active_pgroup(session* ses) {
    return ses->pgroups[ses->pgroup_size - 1];
}
