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



int dist_next_char(char* buf);
bool is_token(char* str);
bool is_yash_cmd(char* str);


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
    int to_next = dist_next_char(buf);
    if (to_next == -1) {
        return -1;
    }

    // first create a new process group
    pgroup* new_pgroup = malloc(sizeof(pgroup));
    pgroup_init(new_pgroup);
    new_pgroup->name = strdup(buf);

    // create a new process
    // in the future, new processes will only be created from pipes
    process* new_process = malloc(sizeof(process));         // allocate for a new process
    process_init(new_process);

    //
    // This while loop handles creating new processes.
    //
    while(1) {   // process until EOF
        to_next = dist_next_char(buf_ptr);
        if (to_next == -1) {    // we're done processing
            session_set_ready_pgroup(ses, new_pgroup);
            return 0;
        }

        buf_ptr += to_next;

        // at this point, we are assured that buf_ptr points to the beginning of a command
        // set pointers to where buf_ptr is right now
        char* arg_begin_ptr = buf_ptr;
        char* arg_end_ptr = arg_begin_ptr;

        //
        // This while loop handles creating new arguments for the process.
        //
        while (1) {
            to_next = dist_next_char(arg_end_ptr);
            if (to_next == -1) {       // we're done processing
                pgroup_insert(new_pgroup, new_process);
                buf_ptr = arg_end_ptr;
                break;
            }
            arg_end_ptr += to_next;
            arg_begin_ptr = arg_end_ptr;

            // at this point, we are guaranteed that arg_end_ptr points to a current argument
            // (we are counting the command itself as an argument)

            if (is_yash_cmd(arg_begin_ptr)) {
                process_destroy(new_process);
                pgroup_destroy(new_pgroup);

                char cur_char = *arg_begin_ptr;

                int wstatus;

                if (cur_char == 'f') {
                    pgroup* bg_pgroup = session_get_recent_bg_pgroup(ses);
                    session_move_to_fg(ses);
                    if (bg_pgroup) {
                        killpg(bg_pgroup->pgid, SIGCONT);
                        if (waitpid(-1 * bg_pgroup->pgid, &wstatus, 0) == -1) {
                            perror("waitpid");
                        }
                    }
                    else {
                        printf("No Jobs\n");
                    }
                    return -1;
                }

                else if (cur_char == 'b') {
                    pgroup* bg_pgroup = session_get_recent_bg_pgroup(ses);
                    if (bg_pgroup) {
                        if (bg_pgroup->state == 'R') {
                            printf("Already running in background\n");
                            return -1;
                        }
                        killpg(bg_pgroup->pgid, SIGCONT);
                        if (waitpid(-1 * bg_pgroup->pgid, &wstatus, WNOHANG) == -1) {
                            perror("waitpid");
                        }
                    }
                    else {
                        printf("No jobs\n");
                    }
                    return -1;
                }

                else if (cur_char == 'j') {
                    session_print_bg_pgroups(ses);
                    return -1;
                }
            }
            // check for special tokens - this indicates we are done with the current command
            if (is_token(arg_begin_ptr)) {
                char cur_char = *arg_begin_ptr;

                // first check if we have a command or not
                if (new_process->arg_size == 0) {
                    printf("yash: %c: token does not have a preceding command.\n", cur_char);
                    process_destroy(new_process);
                    pgroup_destroy(new_pgroup);
                    return -1;
                }

                // REDIRECTS
                // For redirects, we don't take in any more args and instead set the proper flags and filenames for
                // the process.

                if (cur_char == '<') {
                    // go to character immediately after token
                    ++arg_end_ptr;

                    // first check if EOF received
                    to_next = dist_next_char(arg_end_ptr);
                    if (to_next == -1) {
                        printf("yash: must specify filename after <.\n");
                        process_destroy(new_process);
                        pgroup_destroy(new_pgroup);
                        return -1;
                    }

                    // now check for invalid chained token
                    arg_end_ptr += to_next;
                    if (is_token(arg_end_ptr)) {
                        printf("yash: invalid token after <.\n");
                        process_destroy(new_process);
                        pgroup_destroy(new_pgroup);
                        return -1;
                    }

                    new_process->redirect_in = true;
                    char* file_begin_ptr = arg_end_ptr;
                    char* file_end_ptr = file_begin_ptr;
                    uint32_t filename_len = 0;              // we need to know the length of the filename
                    if (*file_begin_ptr == '"') {           // user-provided filename
                        ++file_begin_ptr;
                        ++file_end_ptr;
                        while (*file_end_ptr != '"') {
                            if (*file_end_ptr == '\0') {
                                printf("yash: expected end of filename.\n");
                                process_destroy(new_process);
                                pgroup_destroy(new_pgroup);
                                return -1;
                            }
                            ++filename_len;
                            ++file_end_ptr;
                        }
                    }
                    else {
                        while (*file_end_ptr != ' ' && *file_end_ptr != '\0') {
                            ++filename_len;
                            ++file_end_ptr;
                        }
                    }

                    new_process->redirect_in_filename = malloc(sizeof(char) * (filename_len + 1));
                    for (uint32_t i = 0; i < filename_len; ++i) {
                        new_process->redirect_in_filename[i] = file_begin_ptr[i];
                    }
                    new_process->redirect_in_filename[filename_len] = '\0';    // string sentinel
                    // reset pointers to next argument or EOF
                    arg_end_ptr = file_end_ptr;     // set pointer to end of filename
                }

                else if (cur_char == '>') {
                    // go to character immediately after token
                    ++arg_end_ptr;

                    // first check if EOF received
                    to_next = dist_next_char(arg_end_ptr);
                    if (to_next == -1) {
                        printf("yash: must specify filename after >.\n");
                        process_destroy(new_process);
                        pgroup_destroy(new_pgroup);
                        return -1;
                    }

                    // now check for invalid chained token
                    arg_end_ptr += to_next;
                    if (is_token(arg_end_ptr)) {
                        printf("yash: invalid token after >.\n");
                        process_destroy(new_process);
                        pgroup_destroy(new_pgroup);
                        return -1;
                    }

                    new_process->redirect_out = true;
                    char* file_begin_ptr = arg_end_ptr;
                    char* file_end_ptr = file_begin_ptr;
                    uint32_t filename_len = 0;              // we need to know the length of the filename
                    if (*file_begin_ptr == '"') {           // user-provided filename
                        ++file_begin_ptr;
                        ++file_end_ptr;
                        while (*file_end_ptr != '"') {
                            if (*file_end_ptr == '\0') {
                                printf("yash: expected end of filename.\n");
                                process_destroy(new_process);
                                pgroup_destroy(new_pgroup);
                                return -1;
                            }
                            ++filename_len;
                            ++file_end_ptr;
                        }
                    }
                    else {
                        while (*file_end_ptr != ' ' && *file_end_ptr != '\0') {
                            ++filename_len;
                            ++file_end_ptr;
                        }
                    }

                    new_process->redirect_out_filename = malloc(sizeof(char) * (filename_len + 1));
                    for (uint32_t i = 0; i < filename_len; ++i) {
                        new_process->redirect_out_filename[i] = file_begin_ptr[i];
                    }
                    new_process->redirect_out_filename[filename_len] = '\0';    // string sentinel
                    // reset pointers to next argument or EOF
                    arg_end_ptr = file_end_ptr;     // set pointer to end of filename
                }

                else if (cur_char == '2') {
                    // go to character immediately after token
                    arg_end_ptr += 2;

                    // first check if EOF received
                    to_next = dist_next_char(arg_end_ptr);
                    if (to_next == -1) {
                        printf("yash: must specify filename after 2>.\n");
                        process_destroy(new_process);
                        pgroup_destroy(new_pgroup);
                        return -1;
                    }

                    // now check for invalid chained token
                    arg_end_ptr += to_next;
                    if (is_token(arg_end_ptr)) {
                        printf("yash: invalid token after 2>.\n");
                        process_destroy(new_process);
                        pgroup_destroy(new_pgroup);
                        return -1;
                    }

                    new_process->redirect_err = true;
                    char* file_begin_ptr = arg_end_ptr;
                    char* file_end_ptr = file_begin_ptr;
                    uint32_t filename_len = 0;              // we need to know the length of the filename
                    if (*file_begin_ptr == '"') {           // user-provided filename
                        ++file_begin_ptr;
                        ++file_end_ptr;
                        while (*file_end_ptr != '"') {
                            if (*file_end_ptr == '\0') {
                                printf("yash: expected end of filename.\n");
                                process_destroy(new_process);
                                pgroup_destroy(new_pgroup);
                                return -1;
                            }
                            ++filename_len;
                            ++file_end_ptr;
                        }
                    }
                    else {
                        while (*file_end_ptr != ' ' && *file_end_ptr != '\0') {
                            ++filename_len;
                            ++file_end_ptr;
                        }
                    }

                    new_process->redirect_err_filename = malloc(sizeof(char) * (filename_len + 1));
                    for (uint32_t i = 0; i < filename_len; ++i) {
                        new_process->redirect_err_filename[i] = file_begin_ptr[i];
                    }
                    new_process->redirect_err_filename[filename_len] = '\0';    // string sentinel
                    // reset pointers to next argument or EOF
                    arg_end_ptr = file_end_ptr;     // set pointer to end of filename

                }



                // PIPES
                // For pipes, we need to add the current process to the process group and create a new one.
                //

                else if (cur_char == '|') {
                    // set arg pointer to immediate character after token
                    ++arg_end_ptr;
                    to_next = dist_next_char(arg_end_ptr);
                    if (to_next == -1) {
                        printf("yash: expected argument after |.\n");
                        process_destroy(new_process);
                        pgroup_destroy(new_pgroup);
                        return -1;
                    }
                    arg_end_ptr += to_next;

                    // special case - we need to check if the next arg is a token
                    if (is_token(arg_end_ptr)) {
                        printf("yash: invalid token after |.\n");
                        process_destroy(new_process);
                        pgroup_destroy(new_pgroup);
                        return -1;
                    }

                    // set pipe_out flag to true and insert the process into the current process group
                    new_process->pipe_out = true;
                    pgroup_insert(new_pgroup, new_process);

                    // now create a new process and set the pipe_in flag to true
                    new_process = malloc(sizeof(process));
                    process_init(new_process);
                    new_process->pipe_in = true;

                }


                // BACKGROUND
                // For backgrounding, we just indicate that the process group should be executed in the background.
                // It is assumed that & is the last token in the line.

                else if (cur_char == '&') {
                    new_pgroup->bg = true;
                    ++arg_end_ptr;
                }
            }

            // normal argument
            else {
                uint32_t arg_length = 0;
                while (!is_token(arg_end_ptr) && *arg_end_ptr != ' ' && *arg_end_ptr != '\0') {
                    ++arg_end_ptr;
                    ++arg_length;
                }
                char* new_arg = malloc(sizeof(char) * (arg_length + 1));        // we add 1 for the string sentinel
                for (uint32_t i = 0; i < arg_length; ++i) {
                    new_arg[i] = arg_begin_ptr[i];
                }
                new_arg[arg_length] = '\0';
                process_insert_arg(new_process, new_arg);
            }
        }
    }
}

/**
 * Execute the active process group.
 * @param ses
 */
void line_exec(session* ses) {
    pgroup* ready_pgroup = session_get_ready_pgroup(ses);
    pgroup_exec(ready_pgroup, ses);
}


/**
 * Skips spaces until the next important character/token.
 * @param buf - pointer to the buffer
 * @return the number of characters away from the next token, -1 if there are none
 */
int dist_next_char(char* buf) {
    int count = 0;
    while (*buf == ' ') {
        ++buf;
        ++count;
    }
    if (*buf == '\0') {
        return -1;
    }
    return count;
}


bool is_token(char* str) {
    return  *str == '<' || *str == '>' || (*str == '2' && *(str + 1) == '>') ||
            *str == '|' ||
            *str == '&';
}


bool is_yash_cmd(char* str) {
    return  (*str == 'f' && *(str + 1) == 'g') ||
            (*str == 'b' && *(str + 1) == 'g') ||
            (*str == 'j' && *(str + 1) == 'o' && *(str + 2) == 'b' && *(str + 3) == 's');
}
