//
// Created by bennycooly on 9/11/16.
//
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <fcntl.h>

#include "process.h"
#include "pgroup.h"

void process_init(process* proc) {
    proc->pid = 0;
    proc->ppid = 0;
    proc->pgid = 0;
    proc->state = '\0';
    proc->pipe_in = false;
    proc->pipe_out = false;
    proc->redirect_in = false;
    proc->redirect_out = false;
    proc->redirect_err = false;
    proc->redirect_in_filename = NULL;
    proc->redirect_out_filename = NULL;
    proc->redirect_err_filename = NULL;
    proc->arg_size = 0;
    proc->arg_capacity = MIN_ARGS_CAPACITY;
    proc->args = malloc(sizeof(char*) * proc->arg_capacity);
}

void process_print(process* proc) {
    printf("Command:\t%s\n", proc->args[0]);
    printf("Args:\t\n");
    for (uint32_t i = 0; i < proc->arg_size; ++i) {
        printf("\t\t[%d]: %s\n", i, proc->args[i]);
    }
    printf("pid:\t%d\n", proc->pid);
    printf("ppid:\t%d\n", proc->ppid);
    printf("pgid:\t%d\n", proc->pgid);
    printf("state:\t%c\n", proc->state);
}

void process_exec(process* proc, pgroup* pg, int* fd_pipe, int pipe_index, uint32_t pipe_len) {
    pid_t cpid = fork();
    if (cpid == -1) {          // fork error
        perror("Failed to fork");
    }

    else if (cpid == 0) {    // child
        if (setpgid(0, pg->pgid)) {
            perror("setpgid");
        }
        // printf("child set pgid to %d\n", getpgid(0));
        // check for file redirects
        if (proc->redirect_in) {
            int fd_redir = open(proc->redirect_in_filename, O_RDONLY);
            if (fd_redir == -1) {
                perror("open");
                exit(1);
            }
            if (dup2(fd_redir, STDIN_FILENO) == -1) {
                perror("dup2");
                exit(1);
            }
            if (close(fd_redir) == -1) {
                perror("close");
                exit(1);
            }
        }
        if (proc->redirect_out) {
            int fd_redir = open(proc->redirect_out_filename, O_RDWR | O_CREAT, S_IRWXU);
            if (fd_redir == -1) {
                perror("open");
                exit(1);
            }
            if (dup2(fd_redir, STDOUT_FILENO) == -1) {
                perror("dup2");
                exit(1);
            }
            if (close(fd_redir) == -1) {
                perror("close");
                exit(1);
            }
        }
        if (proc->redirect_err) {
            int fd_redir = open(proc->redirect_err_filename, O_RDWR | O_CREAT, S_IRWXU);
            if (fd_redir == -1) {
                perror("open");
                exit(1);
            }
            if (dup2(fd_redir, STDERR_FILENO) == -1) {
                perror("dup2");
                exit(1);
            }
            if (close(fd_redir) == -1) {
                perror("close");
                exit(1);
            }
        }

        if (proc->pipe_in) {
            if (dup2(fd_pipe[pipe_index-2], STDIN_FILENO) == -1) {
                perror("dup2");
                exit(1);
            }
        }

        if (proc->pipe_out) {
            if (dup2(fd_pipe[pipe_index + 1], STDOUT_FILENO) == -1) {
                perror("dup2");
                exit(1);
            }
        }

        // close all pipes
        for (uint32_t i = 0; i < pipe_len; ++i) {
            if (close(fd_pipe[i]) == -1) {
                perror("close");
                exit(1);
            }
        }

        execvp(proc->args[0], proc->args);
        perror("yash");
        exit(1);
    }

    else {                  // parent
        if (setpgid(cpid, pg->pgid) == -1) {
            perror("setpgid");
        }
        // printf("parent set pgid to %d\n", pg->pgid);
        // set values in process
        proc->pid = cpid;
        proc->ppid = getpid();
        proc->pgid = getpgid(cpid);
        proc->state = 'R';
    }
}


void process_insert_arg(process* proc, char* arg) {
    // check if we need to reallocate
    if (proc->arg_size == proc->arg_capacity - 1) {   // double the capacity; we subtract one because we need one extra arg for exec
        proc->arg_capacity *= 2;
        char** temp = realloc(proc->args, sizeof(char*) * proc->arg_capacity);
        if (!temp) {
            printf("Memory allocation failure.\n");
            exit(1);
        }
        proc->args = temp;
    }
    // first insert, then increment size
    proc->args[proc->arg_size] = arg;
    proc->args[proc->arg_size + 1] = NULL;    // for execvp
    ++(proc->arg_size);
}

void process_destroy(process* proc) {
    if (proc->arg_size > 0) {
        for (uint32_t i = 0; i < proc->arg_size; ++i) {
            free(proc->args[i]);
        }
        free(proc->args);
        proc->args = NULL;
    }
    if (proc->redirect_in_filename) {
        free(proc->redirect_in_filename);
        proc->redirect_in_filename = NULL;
    }
    if (proc->redirect_out_filename) {
        free(proc->redirect_out_filename);
        proc->redirect_out_filename = NULL;
    }
    if (proc->redirect_err_filename) {
        free(proc->redirect_err_filename);
        proc->redirect_err_filename = NULL;
    }
    free(proc);
}