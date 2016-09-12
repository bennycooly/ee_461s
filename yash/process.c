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

void process_exec(process* proc, pgroup* pg, bool is_first) {
    // we use a pipe to have the child wait for the parent to set its pgid
    int fd_pgid[2];
    char pgid_flag = '\0';
    if (pipe(fd_pgid) == -1) {
        perror("pipe");
    }

    pid_t cpid = fork();
    if (cpid < 0) {          // fork error
        perror("Failed to fork");
    }
    else if (cpid == 0) {    // child
        if (close(fd_pgid[1]) == -1) {
            perror("close");
        }
        if (read(fd_pgid[0], &pgid_flag, sizeof(pgid_flag)) == -1) {
            perror("read");
            exit(1);
        }
        printf("child: pgid is now %d\n", getpgid(0));
        // check for file redirects
        if (proc->redirect_in) {
            int fd = open(proc->redirect_in_filename, O_RDONLY);
            if (fd == -1) {
                perror("open");
                exit(1);
            }
            if (dup2(fd, STDIN_FILENO) == -1) {
                perror("dup2");
                exit(1);
            }
            if (close(fd) == -1) {
                perror("close");
                exit(1);
            }
        }
        if (proc->redirect_out) {
            int fd = open(proc->redirect_out_filename, O_RDWR | O_CREAT, S_IRWXU);
            if (fd == -1) {
                perror("open");
                exit(1);
            }
            if (dup2(fd, STDOUT_FILENO) == -1) {
                perror("dup2");
                exit(1);
            }
            if (close(fd) == -1) {
                perror("close");
                exit(1);
            }
        }
        execvp(proc->args[0], proc->args);
        perror("yash");
        exit(1);
    }
    else {                  // parent
        if (is_first) {
            if (setpgid(cpid, 0) == -1) {
                perror("setpgid");
            }
            pg->pgid = cpid;
        }
        else {
            if (setpgid(cpid, pg->pgid) == -1) {
                perror("setpgid");
            }
        }
        if (close(fd_pgid[0]) == -1) {
            perror("close");
        }
        if (write(fd_pgid[1], &pgid_flag, sizeof(pgid_flag)) == -1) {
            perror("write");
        }
        if (close(fd_pgid[1]) == -1) {
            perror("close");
        }
        // set values in process
        proc->pid = cpid;
        proc->ppid = getpid();
        proc->pgid = getpgid(cpid);
        process_print(proc);
        int wstatus;
        waitpid(-1 * proc->pgid, &wstatus, 0);
    }
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