//
// Created by bennycooly on 9/11/16.
//

#include <stdio.h>
#include <unistd.h>
#include <wait.h>

#include "session.h"
#include "pgroup-list.h"
#include "pgroup.h"
#include "process.h"

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
        printf("Setting process state to stopped\n");
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
    uint32_t index = 1;
    while(cur_pg_node) {
        int wstatus;
        if (waitpid(-1 * cur_pg_node->pg->pgid, &wstatus, WNOHANG) == -1) {
            perror("wait");
            printf("[%d]+ Done\t\t%s\n", index, cur_pg_node->pg->name);
            pgroup_list_remove_node(ses->bg_pgroups, cur_pg_node);
        }
        cur_pg_node = cur_pg_node->next;
        ++index;
    }
}

void session_print_bg_pgroups(session* ses) {
    pgroup_node* cur_pg_node = ses->bg_pgroups->first;
    if (!cur_pg_node) {
        printf("No jobs\n");
        return;
    }
    uint32_t index = 1;
    while(cur_pg_node) {
        if (cur_pg_node->pg->state == 'R') {
            printf("[%d]+ Running\t\t%s\n", index, cur_pg_node->pg->name);
        }
        else if (cur_pg_node->pg->state == 'T') {
            printf("[%d]- Stopped\t\t%s\n", index, cur_pg_node->pg->name);
        }

        cur_pg_node = cur_pg_node->next;
        ++index;
    }
}
