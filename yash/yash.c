
#include <stdlib.h>
#include <stdio.h>
// #include <stdint.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_BUFFER_SIZE 256

void read_line(char*);

int main() {
    char buffer[MAX_BUFFER_SIZE];
    while(1) {
        printf("# ");           // print leading prompt
        read_line(buffer);
        pid_t pid = fork();
        if (pid < 0) {
            perror("Failed to fork");
        } else if (pid == 0) {
            char *myargs[4];
            myargs[0] = "ls";
            myargs[1] = "-all";
            myargs[2] = "/home";
            myargs[3] = NULL;
            execvp(myargs[0], myargs);
            perror("Failed to exec");
        } else {
            wait(NULL);
        }
    }
}

void read_line(char* buf) {
    if (fgets(buf, MAX_BUFFER_SIZE, stdin) == NULL) {
        perror("Error reading line");
    }
    size_t len = strlen(buf);
    buf[len - 1] = '\0';
}
