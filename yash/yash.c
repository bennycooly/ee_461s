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
#include <sys/ioctl.h>
#include <fcntl.h>

/**
 * Our headers.
 */
#include "line.h"
#include "command.h"

void sig_handler(int signo);
void init_shell();

int main() {
    init_shell();
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        printf("couldn't catch SIGINT\n");
    }
    if (signal(SIGCHLD, sig_handler) == SIG_ERR) {
        printf("couldn't catch SIGCHLD\n");
    }
    char buffer[MAX_BUFFER_SIZE];
    int pg_status = 1;
    while(1) {
        printf("# ");           // print leading prompt
        line_read(buffer, MAX_BUFFER_SIZE);            // read the next line from user
        line_exec(&pg_status);
        pid_t pid = fork();
        if (pid < 0) {          // fork error
            perror("Failed to fork");
        }
        else if (pid == 0) {    // child
            cmd_execute();
        }
        else {                  // parent
            // wait(NULL);
        }
    }
}

/**
 * Initialize the shell. We need to set ourselves as a new session leader.
 */
void init_shell() {
    setpgid(getpid(), 0);
    printf("yash spawned with:\npid:\t%d\npgid:\t%d\nsid:\t%d\n", getpid(), getpgid(getpid()), getsid(getpid()));
}

void sig_handler(int signo) {
    if (signo == SIGINT) {
        printf("recieved SIGINT\n");
        exit(1);
    }
    if (signo == SIGCHLD) {
        int wstatus;
        pid_t cpid = waitpid(-1, &wstatus, 0);
        printf("child pid %d exited\n", cpid);
    }
}

