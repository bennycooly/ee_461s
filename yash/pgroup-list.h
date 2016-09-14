//
// Created by bennycooly on 9/13/16.
//

#ifndef YASH_PGROUP_LIST_H
#define YASH_PGROUP_LIST_H

typedef struct pgroup pgroup;
typedef struct pgroup_node pgroup_node;

typedef struct pgroup_node {
    pgroup* pg;
    pgroup_node* previous;
    pgroup_node* next;
} pgroup_node ;

typedef struct pgroup_list {
    pgroup_node* first;
} pgroup_list;


void pgroup_node_destroy(pgroup_node* pg_node);

void pgroup_list_insert_pg(pgroup_list* pg_list, pgroup* pg);
void pgroup_list_remove_node(pgroup_list* pg_list, pgroup_node* pg);


#endif //YASH_PGROUP_LIST_H
