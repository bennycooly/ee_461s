//
// Created by bennycooly on 9/11/16.
//

#include <stdio.h>
#include <unistd.h>
#include <wait.h>

#include "session.h"
#include "process.h"
#include "pgroup.h"
#include "pgroup-list.h"

void set_session() {

}

pgroup* session_get_ready_pgroup(session* ses) {
    return ses->ready_pgroup;
}

void session_set_ready_pgroup(session* ses, pgroup* pg) {
    ses->ready_pgroup = pg;
}

pgroup* session_get_fg_pgroup(session* ses) {
    return ses->fg_pgroup;
}

void session_set_fg_pgroup(session* ses, pgroup* pg) {
    ses->fg_pgroup = pg;
}

void session_remove_fg_pgroup(session* ses) {
    pgroup_destroy(ses->fg_pgroup);
    ses->fg_pgroup = NULL;
}

pgroup* session_get_recent_bg_pgroup(session* ses) {
    if (ses->bg_pgroups->first) {
        return ses->bg_pgroups->first->pg;
    }
    return NULL;
}

pgroup_node* session_get_recent_bg_pgroup_node(session* ses) {
    if (ses->bg_pgroups->first) {
        return ses->bg_pgroups->first;
    }
    return NULL;
}

/**
 * We always insert into the front of the linked list.
 * @param ses
 * @param pg
 */
void session_insert_bg_pgroup(session* ses, pgroup* pg) {
    pgroup_list_insert_pg(ses->bg_pgroups, pg);
}


void session_move_to_bg(session* ses) {
    pgroup* fg_pg = ses->fg_pgroup;
    if (fg_pg) {
        fg_pg->state = 'T';
        // printf("Setting process state to stopped\n");
        pgroup_list_insert_pg(ses->bg_pgroups, fg_pg);
        ses->fg_pgroup = NULL;
    }
}

void session_move_to_fg(session* ses) {
    pgroup_node* bg_pg_node = session_get_recent_bg_pgroup_node(ses);
    if (bg_pg_node) {
        ses->fg_pgroup = bg_pg_node->pg;
        pgroup_list_remove_node(ses->bg_pgroups, bg_pg_node);
    }
}

void session_check_update(session* ses) {
    pgroup_node* cur_pg_node = ses->bg_pgroups->first;
    if (!cur_pg_node) {
        return;
    }
    pgroup_node* temp_pg_node = cur_pg_node;
    while (cur_pg_node) {
        temp_pg_node = cur_pg_node;
        cur_pg_node = cur_pg_node->next;
    }
    // set to the last node
    cur_pg_node = temp_pg_node;
    uint32_t index = 1;
    while (cur_pg_node) {
        if (waitpid(cur_pg_node->pg->pgid, NULL, WNOHANG) == -1) {
            if (!cur_pg_node->previous) {
                printf("[%d]+ ", index);
            }
            else {
                printf("[%d]- ", index);
            }
            printf("Done\t\t%s\n", cur_pg_node->pg->name);
            pgroup_list_remove_node(ses->bg_pgroups, cur_pg_node);
        }
        cur_pg_node = cur_pg_node->previous;
        ++index;
    }
}

/**
 * We want to print out our linked list backwards.
 * @param ses
 */
void session_print_bg_pgroups(session* ses) {
    pgroup_node* cur_pg_node = ses->bg_pgroups->first;
    if (!cur_pg_node) {
        printf("yash: no jobs\n");
        return;
    }
    pgroup_node* temp_pg_node = cur_pg_node;
    while (cur_pg_node) {
        temp_pg_node = cur_pg_node;
        cur_pg_node = cur_pg_node->next;
    }
    // set to the last node
    cur_pg_node = temp_pg_node;
    uint32_t index = 1;
    while (cur_pg_node) {
        if (!cur_pg_node->previous) {
            printf("[%d]+ ", index);
        }
        else {
            printf("[%d]- ", index);
        }
        if (cur_pg_node->pg->state == 'R') {
            printf("Running\t\t%s\n", cur_pg_node->pg->name);
        }
        else if (cur_pg_node->pg->state == 'T') {
            printf("Stopped\t\t%s\n", cur_pg_node->pg->name);
        }

        cur_pg_node = cur_pg_node->previous;
        ++index;
    }
}

/**
 * Similar to printing all jobs except we just print the matched pgroup.
 * @param ses
 * @param pg
 */
void session_print_cur_bg_pgroup(session* ses) {
    pgroup_node* cur_pg_node = session_get_recent_bg_pgroup_node(ses);
    printf("[%d]+ Running\t\t%s\n", ses->bg_pgroups->size, cur_pg_node->pg->name);
}
