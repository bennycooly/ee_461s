//
// Created by bennycooly on 9/11/16.
//
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <fcntl.h>

#include "process.h"
#include "pgroup.h"

void process_print(process* proc){
    printf("Command:\t%s\n", proc->cmd);
    printf("Args:\t\n");
    for (uint32_t i = 0; i < proc->arg_size; ++i) {
        printf("\t\t[%d]: %s\n", i, proc->args[i]);
    }
    printf("pid:\t%d\n", proc->pid);
    printf("ppid:\t%d\n", proc->ppid);
    printf("pgid:\t%d\n", proc->pgid);
}

void process_exec(process* proc, pgroup* pg, bool is_first) {
    // check for pipes and/or file redirects
//    if (proc->redirect_in) {
//        int fd = open(proc->redirect_in_filename, O_RDONLY);
//        if (fd == -1) {
//            perror("open");
//            exit(1);
//        }
//        if (dup2(fd, STDIN_FILENO) == -1) {
//            perror("dup2");
//            exit(1);
//        }
//        if (close(fd) == -1) {
//            perror("close");
//            exit(1);
//        }
//    }
    pid_t cpid = fork();
    if (cpid < 0) {          // fork error
        perror("Failed to fork");
    }
    else if (cpid == 0) {    // child
        if (is_first) {     // we make ourselves the group leader
            setpgid(0, 0);
        }
        else {
            setpgid(cpid, pg->pgid);
        }
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
        execvp(proc->cmd, proc->args);
        perror("yash");
        exit(1);
    }
    else {                  // parent
        if (is_first) {
            pg->pgid = cpid;
        }
        proc->pid = cpid;
        proc->ppid = getpid();
        proc->pgid = pg->pgid;
        process_print(proc);
        int wstatus;
        waitpid(-1 * pg->pgid, &wstatus, 0);
    }
}

void process_free(process* proc) {
    if (proc->arg_size > 0) {
        free(proc->args);
        proc->args = NULL;
    }
    free(proc);
}