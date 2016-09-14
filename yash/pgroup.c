//
// Created by bennycooly on 9/11/16.
//

#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <errno.h>

#include "pgroup.h"
#include "process.h"
#include "session.h"

void pgroup_sig_handler(int signo);

// signal flag
volatile sig_atomic_t sig_received;

void pgroup_init(pgroup* pg) {
    pg->pgid = 0;
    pg->state = '\0';
    pg->name = NULL;
    pg->size = 0;
    pg->bg = false;
    pg->capacity = MIN_PGROUP_CAPACITY;
    pg->processes = malloc(sizeof(process*) * pg->capacity);
}

void pgroup_exec(pgroup* pg, session* ses) {
    // we first fork to create a new process - this will be our process group leader

    // now we need a way to signal that we are done forking the children
    struct sigaction sa;
    sa.sa_handler = pgroup_sig_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
    }

    pid_t ppid = getpid();

    pid_t cpid = fork();

    if (cpid == -1) {
        perror("fork");
        exit(1);
    }

    // execute all the child processes
    else if (cpid == 0) {

        if (setpgid(0, 0) == -1) {
            perror("setpgid");
            exit(1);
        }
        pg->pgid = getpgid(getpid());
        // printf("New process group created with pgid %d\n", pg->pgid);

        // set our signals
//        struct sigaction sa;
//        sa.sa_handler = pgroup_sig_handler;
//        sa.sa_flags = 0;
//        sigemptyset(&sa.sa_mask);
        if (sigaction(SIGINT, &sa, NULL) == -1) {
            perror("sigaction");
        }
        if (sigaction(SIGTSTP, &sa, NULL) == -1) {
            perror("sigaction");
        }
        if (sigaction(SIGCHLD, &sa, NULL) == -1) {
            perror("sigaction");
        }
        if (sigaction(SIGCONT, &sa, NULL) == -1) {
            perror("sigaction");
        }

        // create pipes for our processes
        uint32_t num_pipes = 0;
        for (uint32_t i = 0; i < pg->size; ++i) {   // just need to count the number of pipe outs
            if (pg->processes[i]->pipe_out) {
                ++num_pipes;
            }
        }
        int* fd = malloc(sizeof(int) * 2 * num_pipes);

        for (uint32_t i = 0; i < num_pipes; ++i) {
            if (pipe(fd + (2 * i)) == -1) {
                perror("pipe");
                exit(1);
            }
        }

        uint32_t pipe_index = 0;

        for (uint32_t i = 0; i < pg->size; ++i) {
            if (pg->processes[i]->pipe_in || pg->processes[i]->pipe_out) {
                process_exec(pg->processes[i], pg, fd, pipe_index, 2 * num_pipes);
                pipe_index += 2;                // increment pipe_index by 2
            }
            else {
                process_exec(pg->processes[i], pg, NULL, 0, 0);
            }

        }

        // close all the pipes
        for (uint32_t i = 0; i < (2 * num_pipes); ++i) {
            if (close(fd[i]) == -1) {
                perror("close");
                exit(1);
            }
        }

        // now indicate to the parent that we're done forking all the children
        fflush(stdout);
        kill(ppid, SIGUSR1);
        // printf("finished forking all children\n");

        int wstatus;

        if (waitpid(-1 * pg->pgid, &wstatus, 0) == -1) {
            perror("pgroup waitpid");
            if (errno == EINTR) {
                printf("signal received\n");
                switch (sig_received) {
                    case SIGINT:
                        printf("child received SIGINT\n");
                        for (uint32_t i = 0; i < pg->size; ++i) {
                            if (kill(pg->processes[i]->pid, SIGINT) == -1) {
                                perror("kill");
                            }
                            pg->processes[i]->state = 'T';
                        }
                        sig_received = 0;
                        exit(0);

                    case SIGTSTP:
                        printf("child received SIGTSTP\n");
                        for (uint32_t i = 0; i < pg->size; ++i) {
                            if (kill(pg->processes[i]->pid, SIGTSTP) == -1) {
                                perror("kill");
                            }
                            pg->processes[i]->state = 'T';
                        }
                        while (sig_received != SIGCONT) {}
                        printf("all children finished from SIGCONT\n");
                        for (uint32_t i = 0; i < pg->size; ++i) {
                            if (kill(pg->processes[i]->pid, SIGCONT) == -1) {
                                perror("kill");
                            }
                            pg->processes[i]->state = 'R';
                        }
                        if (waitpid(-1 * pg->pgid, &wstatus, 0) == -1) {
                            perror("pgroup waitpid");
                        }

                        sig_received = 0;
                        exit(0);
                        break;

                    case SIGCONT:
                        printf("child received SIGCONT\n");
                        for (uint32_t i = 0; i < pg->size; ++i) {
                            if (kill(pg->processes[i]->pid, SIGCONT) == -1) {
                                perror("kill");
                            }
                            pg->processes[i]->state = 'R';
                        }
                        if (waitpid(-1 * pg->pgid, &wstatus, 0) == -1) {
                            perror("pgroup waitpid");
                        }
                        // printf("all children finished from SIGCONT\n");
                        exit(0);

                    default:
                        sig_received = 0;
                        break;
                }
            }

        }

        // printf("wstatus is %d\n", wstatus);
        free(fd);
        // printf("all children are done\n");
        exit(0);
    }

    else {
        // set our pgid
        pg->pgid = cpid;
        pg->state = 'R';
        // set values in process
        for (uint32_t i = 0; i < pg->size; ++i) {
            pg->processes[i]->pgid = cpid;
            pg->processes[i]->state = 'R';
        }

        // wait for child to finish forking all processes
        while (sig_received != SIGUSR1) {}
        sig_received = 0;

        int wstatus;
        if (pg->bg) {
            session_insert_bg_pgroup(ses, pg);
            if (waitpid(-1 * pg->pgid, &wstatus, WNOHANG) == -1) {
                perror("waitpid");
            }
        }
        else {
            session_set_fg_pgroup(ses, pg);
            if (waitpid(-1 * pg->pgid, &wstatus, 0) == -1) {
                perror("waitpid");
            }
            // printf("pgroup finished\n");
            // session_remove_fg_pgroup(ses);
        }

    }

}

void pgroup_insert(pgroup* pg, process* proc) {
    // check if we need to reallocate
    if (pg->size == pg->capacity) {   // double the capacity and reallocate
        pg->capacity *= 2;
        process** temp = realloc(pg->processes, sizeof(process*) * pg->capacity);
        if (!temp) {
            printf("Memory allocation failure.\n");
            exit(1);
        }
        pg->processes = temp;
    }
    pg->processes[pg->size] = proc;
    ++(pg->size);
}

void pgroup_destroy(pgroup* pg) {
    if (pg->size > 0) {
        for (uint32_t i = 0; i < pg->size; ++i) {
            free(pg->processes[i]);
        }
        free(pg->processes);
        pg->processes = NULL;
    }
    if (pg->name) {
        free(pg->name);
        pg->name = NULL;
    }
    free(pg);
}

void pgroup_sig_handler(int signo) {
    if (signo == SIGINT) {
        printf("PG received SIGINT\n");
        sig_received = SIGINT;
    }
    else if (signo == SIGTSTP) {
        printf("PG received SIGTSTP\n");
        sig_received = SIGTSTP;
    }
    else if (signo == SIGCONT) {
        printf("PG received SIGCONT\n");
        sig_received = SIGCONT;
    }
    else if (signo == SIGUSR1) {
        printf("PG received SIGUSR1\n");
        sig_received = SIGUSR1;
    }
}
