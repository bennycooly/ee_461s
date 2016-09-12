//
// Created by bennycooly on 9/10/16.
//

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <wait.h>

#include "line.h"
#include "process.h"
#include "pgroup.h"
#include "session.h"


/**
 * Read a line from stdin into a buffer.
 */
int line_read(char* buf, uint32_t max_buf_size) {
    if (fgets(buf, max_buf_size, stdin) == NULL) {    // exit if EOF
        // differentiate between EOF and a signal
        if (feof(stdin)) {
            printf("Bye.\n");
            exit(0);
        }
        if (errno == EINTR) {
            return -1;
        }

    }

    size_t len = strlen(buf);            //
    buf[len - 1] = '\0';                 // change the newline to string sentinel
    return 0;
}


/**
 * Parse the buffer and dynamically allocate memory for the commands.
 *
 * @return -1 if line is empty or the command failed.
 */
int line_parse(char* buf, session* ses) {
    char* buf_ptr = buf;
    // check for empty line and skip leading whitespace
    while(*buf_ptr == ' ') {
        ++buf_ptr;
    }
    if (*buf_ptr == '\0') { return -1; }                    // if buffer is 0 at this point then just return

    // first create a new process group
    pgroup* new_pgroup = malloc(sizeof(pgroup));
    pgroup_init(new_pgroup);
    new_pgroup->name = strdup(buf);

    // At this point, we are assured that buf_ptr points to the beginning of a command.
    while(1) {   // process until EOF
        if (*buf_ptr == '\0') { // our parse function will set buf_ptr to the sentinel if we are done
            break;
        }

        // create a new process
        process* new_process = malloc(sizeof(process));         // allocate for a new process
        process_init(new_process);
        char* cmd_ptr = buf_ptr;                                // start looking at where the buffer currently is

        // set pointers to where buf_ptr is right now
        char* arg_begin_ptr = buf_ptr;
        char* arg_end_ptr = arg_begin_ptr;
        uint32_t arg_length = 0;

        // at this point, we are guaranteed that arg_end_ptr points to a current argument (we are counting the command
        // itself as an argument)
        while (1) {
            char cur_char = *arg_end_ptr;
            if (cur_char == '\0') {
                if (arg_begin_ptr != arg_end_ptr) {             // we still need to process the last argument
                    char* new_arg = malloc(sizeof(char) * (arg_length + 1));        // we add 1 for the string sentinel
                    for (uint32_t i = 0; i < arg_length; ++i) {
                        new_arg[i] = arg_begin_ptr[i];
                    }
                    new_arg[arg_length] = '\0';
                    // check if we need to reallocate
                    if (new_process->arg_size == new_process->arg_capacity - 1) {   // double the capacity and reallocate
                        new_process->arg_capacity *= 2;
                        char** temp = realloc(new_process->args, sizeof(char*) * new_process->arg_capacity);
                        if (!temp) {
                            printf("Memory allocation failure.\n");
                            exit(1);
                        }
                        new_process->args = temp;
                    }
                    // first insert, then increment size
                    new_process->args[new_process->arg_size] = new_arg;
                    new_process->args[new_process->arg_size + 1] = NULL;    // for execvp
                    ++(new_process->arg_size);
                }
                buf_ptr = arg_end_ptr;                                  // set buf to exit from main loop
                break;
            }
            if (cur_char == ' ') {                                      // add current arg to process
                char* new_arg = malloc(sizeof(char) * (arg_length + 1));    // we add 1 for the string sentinel
                for (uint32_t i = 0; i < arg_length; ++i) {
                    new_arg[i] = arg_begin_ptr[i];
                }
                new_arg[arg_length] = '\0';

                while (*arg_end_ptr == ' ') {                               // skip spaces
                    ++arg_end_ptr;
                }
                if (*arg_end_ptr == '\0') {                                 // we're done with input
                    buf_ptr = arg_end_ptr;
                    break;
                }
                arg_begin_ptr = arg_end_ptr;
                arg_length = 0;
                // check if we need to reallocate
                if (new_process->arg_size == new_process->arg_capacity - 1) {   // double the capacity and reallocate
                    new_process->arg_capacity *= 2;
                    char** temp = realloc(new_process->args, sizeof(char*) * new_process->arg_capacity);
                    if (!temp) {
                        printf("Memory allocation failure.\n");
                        exit(1);
                    }
                    new_process->args = temp;
                }
                // first insert, then increment size
                new_process->args[new_process->arg_size] = new_arg;
                new_process->args[new_process->arg_size + 1] = NULL;        // for execvp
                ++(new_process->arg_size);
                continue;
            }
            // check for special tokens - this indicates we are done with the current command
            if (cur_char == '>' || cur_char == '<' || cur_char == '2' || cur_char == '|' || cur_char == '&') {

                // first check if we have a command or not
                if (new_process->arg_size == 0) {
                    printf("yash: %c: token does not have a preceding command.\n", cur_char);
                    process_destroy(new_process);
                    pgroup_destroy(new_pgroup);
                    return -1;
                }

                if (cur_char == '<') {
                    if (arg_end_ptr[1] == ' ') {    // next argument MUST be the filename
                        new_process->redirect_in = true;
                        char* file_ptr = arg_end_ptr + 2;
                        while (*file_ptr == ' ') {  // skip any whitespace
                            ++file_ptr;
                        }
                        if (*file_ptr == '\0') {
                            printf("yash: must specify filename after <.\n");
                            process_destroy(new_process);
                            pgroup_destroy(new_pgroup);
                            return -1;
                        }
                        uint32_t filename_len = 0;              // we need to know the length of the filename
                        char* temp_ptr = file_ptr;
                        while (*temp_ptr != ' ' && *temp_ptr != '\0') {
                            ++filename_len;
                            ++temp_ptr;
                        }
                        new_process->redirect_in_filename = malloc(sizeof(char) * (filename_len + 1));
                        for (uint32_t i = 0; i < filename_len; ++i) {
                            new_process->redirect_in_filename[i] = file_ptr[i];
                        }
                        new_process->redirect_in_filename[filename_len] = '\0';    // string sentinel
                        // reset pointers to next argument or EOF
                        arg_end_ptr = temp_ptr;     // set pointer to end of filename
                        while (*arg_end_ptr == ' ') {
                            ++arg_end_ptr;
                        }
                        if (*arg_end_ptr == '\0') { // we reached EOF
                            buf_ptr = arg_end_ptr;
                            break;
                        }
                        arg_begin_ptr = arg_end_ptr;
                        arg_length = 0;
                        continue;
                    }
                    else {
                        printf("yash: invalid token after <.\n");
                        process_destroy(new_process);
                        pgroup_destroy(new_pgroup);
                        return -1;
                    }
                }

                else if (cur_char == '>') {
                    if (arg_end_ptr[1] == ' ') {    // next argument MUST be the filename
                        new_process->redirect_out = true;
                        char* file_ptr = arg_end_ptr + 2;
                        while (*file_ptr == ' ') {  // skip any whitespace
                            ++file_ptr;
                        }
                        if (*file_ptr == '\0') {
                            printf("yash: must specify filename after >.\n");
                            process_destroy(new_process);
                            pgroup_destroy(new_pgroup);
                            return -1;
                        }
                        uint32_t filename_len = 0;              // we need to know the length of the filename
                        char* temp_ptr = file_ptr;
                        while (*temp_ptr != ' ' && *temp_ptr != '\0') {
                            ++filename_len;
                            ++temp_ptr;
                        }
                        new_process->redirect_out_filename = malloc(sizeof(char) * (filename_len + 1));
                        for (uint32_t i = 0; i < filename_len; ++i) {
                            new_process->redirect_out_filename[i] = file_ptr[i];
                        }
                        new_process->redirect_out_filename[filename_len] = '\0';    // string sentinel
                        // reset pointers to next argument or EOF
                        arg_end_ptr = temp_ptr;     // set pointer to end of filename
                        while (*arg_end_ptr == ' ') {
                            ++arg_end_ptr;
                        }
                        if (*arg_end_ptr == '\0') { // we reached EOF
                            buf_ptr = arg_end_ptr;
                            break;
                        }
                        arg_begin_ptr = arg_end_ptr;
                        arg_length = 0;
                        continue;
                    }
                    else {
                        printf("yash: invalid token after >.\n");
                        process_destroy(new_process);
                        pgroup_destroy(new_pgroup);
                        return -1;
                    }
                }
            }
            ++arg_end_ptr;
            ++arg_length;
        }
        new_pgroup->processes[new_pgroup->size] = new_process;
        ++(new_pgroup->size);
    }
    new_pgroup->state = 'R';
    ses->pgroups[0] = new_pgroup;
    return 0;
}

/**
 * Execute the active process group.
 * @param ses
 */
void line_exec(session* ses) {
    pgroup* active_pgroup = session_get_active_pgroup(ses);
    pgroup_exec(active_pgroup);
}
