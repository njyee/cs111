// UCLA CS 111 Lab 1 command execution

// Copyright 2012-2014 Paul Eggert.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "command.h"
#include "command-internals.h"

#include <errno.h>
#include <error.h>
#include <stdlib.h>
#include <string.h>   // for strcmp
#include <unistd.h>
#include <sys/wait.h> // I think for WEXITSTATUS macro
#include <fcntl.h>

#define BYTE_LIMIT 1024

void set_io(command_t c) {
    int infile_desc;
    int outfile_desc;

    if(c->input != NULL) {
        infile_desc = open(c->input, O_RDONLY);
        dup2(infile_desc, 0);
    }
    if(c->output != NULL) {
        //outfile_desc = open(c->output, O_WRONLY | O_APPEND | O_CREAT);
        outfile_desc = open(c->output, O_WRONLY | O_CREAT, 0644);
        //outfile_desc = open(c->output, O_WRONLY | O_CREAT);
        dup2(outfile_desc, 1);
    }
}

void propagate_io(command_t c) {
    int i = 0;
    if(c->input != NULL)
        while(i < 3 && c->u.command[i] != NULL)
            c->u.command[i++]->input = c->input;
    i = 0;
    if(c->output != NULL)
        while(i < 3 && c->u.command[i] != NULL)
            c->u.command[i++]->output = c->output;
}
   
void execute_switch(command_t c); // function prototype

/* TODO:
    * Test
    * Handle if/then/else output better (not essential)
    * Remove unnecessary code/comments
    * Sue Big Data Systems Inc
*/

/* Each command type gets its own "execute_...._command" function */

/* Execute a simple command */

void
execute_simple_command(command_t c) {
    pid_t p = fork();
    int exit_status;
    
    if(p < 0)
        error(1, errno, "fork failed");
    else if(p == 0) {
        set_io(c);
        
        if(!strcmp(c->u.word[0], "exec"))
            execvp(c->u.word[1], c->u.word+1);
        else
            execvp(c->u.word[0], c->u.word);
        
        error(1, errno, "execvp should never return");
    }
    else {
        waitpid(p, &exit_status, 0);
        c->status = WEXITSTATUS(exit_status);
    }
}


/* Execute an if command
    if a; then b; fi > file */

void
execute_if_command(command_t c) {
    pid_t p;
    int exit_status;
    
    propagate_io(c);
    
    p = fork();
    if(p < 0)
        error(1, errno, "fork failed");
    else if(p == 0) { // child
        execute_switch(c->u.command[0]);
        _exit(c->u.command[0]->status);
    } else { // parent
        waitpid(p, &exit_status, 0);
        p = fork();
        if(p < 0)
            error(1, errno, "fork failed");
        else if(p == 0) {
            if(exit_status == 0) { // success
                execute_switch(c->u.command[1]);
                _exit(c->u.command[1]->status);
            } else if(c->u.command[2] != NULL) {
                execute_switch(c->u.command[2]);
                _exit(c->u.command[2]->status);
            }
        } else {
            waitpid(p, &exit_status, 0);
            c->status = WEXITSTATUS(exit_status);
        }
    }
}


/* Execute a pipe command */

void
execute_pipe_command(command_t c) {
    pid_t first_pid;
    pid_t second_pid;
    int buffer[2];
    int exit_status_1;
    int exit_status_2;
    
    if(pipe(buffer) < 0 ) {
        error(1, errno, "pipe creation failed");
    }
    first_pid = fork(); // left side
    if(first_pid < 0) {
        error(1, errno, "fork failed for first_pid");
    } else if(first_pid == 0) {
        close(buffer[0]); // close unused read end
        if(dup2(buffer[1], 1) < 0) {
            error(1, errno, "dup2 failed for first_pid");
        }
        execute_switch(c->u.command[0]);
        _exit(c->u.command[0]->status);
    }

    second_pid = fork();
    if(second_pid < 0) {
        error(1, errno, "fork failed for second_pid");
    } else if(second_pid == 0) {
        close(buffer[1]); //close unused write end
        if(dup2(buffer[0], 0) < 0) {
            error(1, errno, "dup2 failed for second_pid");
        }
        execute_switch(c->u.command[1]);
        _exit(c->u.command[1]->status);
    }
    
    waitpid(first_pid, &exit_status_1, 0);
    close(buffer[1]); // done writing to pipe
    waitpid(second_pid, &exit_status_2, 0);
    close(buffer[0]);
    c->status = WEXITSTATUS(exit_status_2);
}


/* Execute a sequence command */

void
execute_sequence_command(command_t c) {
    pid_t left_pid;
    pid_t right_pid;
    int exit_status;
    
    left_pid = fork();
    if(left_pid < 0) {
        error(1, errno, "fork failed");
    } else if(left_pid == 0) {
        execute_switch(c->u.command[0]);
        _exit(c->u.command[0]->status);
    } else {
        waitpid(left_pid, &exit_status, 0);
        
        if(exit_status == 0) {
            right_pid = fork();
            if(right_pid < 0) {
                error(1, errno, "fork failed");
             } else if(right_pid == 0) {
                execute_switch(c->u.command[1]);
                _exit(c->u.command[1]->status);
            } else {
                waitpid(right_pid, &exit_status, 0);
            }
        }
        
        c->status = WEXITSTATUS(exit_status);
    }
}


/* Execute a subshell command.
   For our purposes, executing a subshell command is essentially
   executing a simple command but independent from other processes */
  
void
execute_subshell_command(command_t c) {
    pid_t p;
    int exit_status;
    
    p = fork();
    if (p<0)
        error(1, errno, "first fork failed");
    else if(p == 0)
    {
        p = fork();
        if(p < 0)
            error(1, errno, "second fork failed");
        else if(p == 0) { 
            set_io(c);
            execute_switch(c->u.command[0]);
            _exit(c->u.command[0]->status);
        } else {
            waitpid(p, &exit_status, 0);
            // c->status = WEXITSTATUS(exit_status);
            _exit(WEXITSTATUS(exit_status));
        }
    } else {
        waitpid(p, &exit_status, 0);
        c->status = WEXITSTATUS(exit_status);
    }
}


/* Execute an until command */

void
execute_until_command(command_t c) {
    pid_t p;
    int exit_status = -1;
    
    propagate_io(c);

    while (1) {
        p = fork();
        if(p < 0)
            error(1, errno, "fork failed");
        else if(p == 0) {
            execute_switch(c->u.command[0]);
            _exit(c->u.command[0]->status);
        } else {
            waitpid(p, &exit_status, 0);
            if(exit_status != 0) {
                p = fork();
                if(p < 0)
                    error(1, errno, "fork failed");
                else if(p == 0) {
                    execute_switch(c->u.command[1]);
                    _exit(c->u.command[1]->status);
                } else {
                    waitpid(p, &exit_status, 0);
                    if (exit_status != 0) {
                        c->status = WEXITSTATUS(exit_status);
                        break;
                    }
                }
            }
            else {
                c->status = WEXITSTATUS(0);
                break;
            }
        }
    }
}


/* Execute a while command */

void
execute_while_command(command_t c) {
    pid_t p;
    int exit_status = -1;
    
    propagate_io(c);

    while (1) {
        p = fork();
        if(p < 0)
            error(1, errno, "fork failed");
        else if(p == 0) {
            execute_switch(c->u.command[0]);
            _exit(c->u.command[0]->status);
        } else {
            waitpid(p, &exit_status, 0);
            if(exit_status == 0) {
                p = fork();
                if(p < 0)
                    error(1, errno, "fork failed");
                else if(p == 0) {
                    execute_switch(c->u.command[1]);
                    _exit(c->u.command[1]->status);
                } else {
                    waitpid(p, &exit_status, 0);
                    if (exit_status != 0) {
                        c->status = WEXITSTATUS(exit_status);
                        break;
                    }
                }
            }
            else {
                c->status = WEXITSTATUS(0);
                break;
            }
        }
    }
}
   

/* switch statement that figures out what kind of command needs to be 
   executed and executes it. */
void
execute_switch(command_t c) {
    switch(c->type)
    {
        case SIMPLE_COMMAND:
            execute_simple_command(c);
            break;
        case IF_COMMAND:
            execute_if_command(c);
            break;
        case PIPE_COMMAND:
            execute_pipe_command(c);
            break;
        case SEQUENCE_COMMAND:
            execute_sequence_command(c);
            break;
        case SUBSHELL_COMMAND:
            execute_subshell_command(c);
            break;
        case UNTIL_COMMAND:
            execute_until_command(c);
            break;
        case WHILE_COMMAND:
            execute_while_command(c);
            break;
        default:
            error(1, errno, "invalid command type passed to execute_switch");
            break;
    }
    
}


int
prepare_profiling (char const *name)
{
  error (0, 0, "warning: profiling not yet implemented");
  return -1;
}

int
command_status (command_t c)
{
  return c->status;
}

void
execute_command (command_t c, int profiling)
{
  execute_switch(c);
}
