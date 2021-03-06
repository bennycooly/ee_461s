/**
 * System headers.
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

/**
 * Our headers.
 */
#include "line.h"
#include "session.h"
#include "pgroup.h"

#define RESET   "\033[0m"
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */

void sig_handler(int signo);
void init_shell(session* new_session);

// signal flag
volatile sig_atomic_t sig_received;

int main() {




    session yash_session;
    init_shell(&yash_session);
    struct sigaction sa;
    sa.sa_handler = sig_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
    }
    if (sigaction(SIGTSTP, &sa, NULL) == -1) {
        perror("sigaction");
    }
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
    }
    char buffer[MAX_BUFFER_SIZE];
    while(1) {
        bool show_prompt = true;
        pgroup* fg_pgroup = session_get_fg_pgroup(&yash_session);
        switch(sig_received) {
            case SIGINT:
                // printf("recognizing sigint received\n");
                if (fg_pgroup) {
                    killpg(fg_pgroup->pgid, SIGINT);
                    show_prompt = false;
                }
                fputc('\n', stdout);
                sig_received = 0;
                break;
            case SIGTSTP:
                // printf("recognizing sigtstp received\n");
                if (fg_pgroup) {
                    killpg(fg_pgroup->pgid, SIGTSTP);
                    session_move_to_bg(&yash_session);
                    // printf("%s moved to bg\n", fg_pgroup->name);
                }
                fputc('\n', stdout);
                sig_received = 0;
                break;
            case SIGCHLD:
                // printf("recognizing sigchld received\n");
                if (fg_pgroup) {
                    session_remove_fg_pgroup(&yash_session);
                }
                else {
                    show_prompt = false;
                }
                sig_received = 0;
                break;
            default:
                break;
        }

        if (show_prompt) {
            printf(BOLDCYAN "# " RESET);           // print leading prompt
        }

        if (line_read(buffer, MAX_BUFFER_SIZE) == -1) {     // read the next line from user
            // session_check_update(&yash_session);
            continue;
        }
        session_check_update(&yash_session);
        if (line_parse(buffer, &yash_session) == -1) {      // if we received an empty line, then continue
            continue;
        }
        line_exec(&yash_session);
    }
}

/**
 * Initialize the shell. We need to set ourselves as a new session leader.
 */
void init_shell(session* new_session) {
    if (setpgid(0, 0) == -1) {
        perror("setpgid");
    }
    if (tcsetpgrp(STDIN_FILENO, getpgid(getpid())) == -1) {
        perror("tcsetpgrp");
    }
    // printf("yash spawned with:\npid:\t%d\npgid:\t%d\nsid:\t%d\n", getpid(), getpgid(getpid()), getsid(getpid()));
    new_session->sid = getpid();
    new_session->ready_pgroup = NULL;
    new_session->fg_pgroup = NULL;
    new_session->bg_pgroups = malloc(sizeof(pgroup_list));
    new_session->bg_pgroups->first = NULL;
}

void sig_handler(int sig) {
    if (sig == SIGINT) {
        // printf("recieved SIGINT\n");
        sig_received = SIGINT;
    }
    if (sig == SIGTSTP) {
        // printf("received SIGTSTP\n");
        sig_received = SIGTSTP;
    }
    if (sig == SIGCHLD) {
        sig_received = SIGCHLD;
    }
}

