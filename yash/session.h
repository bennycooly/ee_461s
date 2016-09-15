//
// Created by bennycooly on 9/11/16.
//

#ifndef YASH_JOB_CONTROL_H
#define YASH_JOB_CONTROL_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

#include "pgroup-list.h"
#include "pgroup.h"


static const uint32_t   MIN_SESSION_CAPACITY     = 5;        // default # of pgroups in session



typedef struct pgroup pgroup;
typedef struct pgroup_list pgroup_list;
typedef struct bg_pgroup_node bg_pgroup_node;
/**
 * Our session struct is basically a stack for process groups.
 *
 * We still need a pointer to the end of the stack so that
 */


typedef struct session {
    pid_t sid;
    pgroup* ready_pgroup;
    pgroup* fg_pgroup;
    // pgroup** bg_pgroups;
    pgroup_list* bg_pgroups;
    // pgroup* pgroups[1];
    // uint32_t bg_pgroup_size;
    // uint32_t bg_pgroup_capacity;
} session;




/**
 * Functions.
 */
void set_session();

pgroup* session_get_ready_pgroup(session*);
void session_set_ready_pgroup(session*, pgroup*);

pgroup* session_get_fg_pgroup(session*);
void session_set_fg_pgroup(session*, pgroup*);

void session_remove_fg_pgroup(session*);

pgroup* session_get_recent_bg_pgroup(session*);
void session_insert_bg_pgroup(session*, pgroup*);

void session_move_to_bg(session* ses);
void session_move_to_fg(session* ses);

void session_check_update(session*);

void session_print_bg_pgroups(session*);
void session_print_cur_bg_pgroup(session*);

#endif //YASH_JOB_CONTROL_H
