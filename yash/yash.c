/**
 * System headers.
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>

/**
 * Our headers.
 */
#include "command.h"

void sig_handler(int signo);

int main() {
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        printf("couldn't catch SIGINT\n");
    }
    while(1) {
        printf("# ");           // print leading prompt
        cmd_read_line();
        pid_t pid = fork();
        if (pid < 0) {
            perror("Failed to fork");
        } else if (pid == 0) {
            cmd_execute();
        } else {
            wait(NULL);
        }
    }
}

void sig_handler(int signo) {
    if (signo == SIGINT) {
        printf("recieved SIGINT\n");
        exit(1);
    }
}

