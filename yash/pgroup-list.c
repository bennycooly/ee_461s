//
// Created by bennycooly on 9/13/16.
//

#include <stdlib.h>

#include "pgroup-list.h"

void pgroup_node_destroy(pgroup_node* pg_node) {
    if (pg_node->previous && pg_node->next) {       // link our prevoius to our next
        pg_node->previous->next = pg_node->next;
        pg_node->next->previous = pg_node->previous;
    }
    else if (pg_node->previous) {
        pg_node->previous->next = NULL;
    }
    else if (pg_node->next) {
        pg_node->next->previous = NULL;
    }
    free(pg_node);
}

void pgroup_list_insert_pg(pgroup_list* pg_list, pgroup* pg) {
    pgroup_node* new_pg_node = malloc(sizeof(pgroup_node));
    new_pg_node->pg = pg;
    new_pg_node->previous = NULL;
    new_pg_node->next = NULL;
    if (!pg_list->first) {
        pg_list->first = new_pg_node;
    }
    else {
        new_pg_node->next = pg_list->first;
        pg_list->first->previous = new_pg_node;
        pg_list->first = new_pg_node;
    }
    ++pg_list->size;
}

void pgroup_list_remove_node(pgroup_list* pg_list, pgroup_node* pg_node) {
    if (pg_list->first == pg_node) {    // handle the special case where we are removing the first node
        pg_list->first = pg_list->first->next;
    }
    pgroup_node_destroy(pg_node);
    --pg_list->size;
}

