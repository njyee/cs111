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

#include <error.h>
#include <stdlib.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */
   
   void execute_switch(command_t c); // function prototype
   
/* Each command type gets its own "execute_...._command" function */

/* Execute a pipe command */
void
execute_pipe_command(command_t c) {
    pid_t returned_pid;
    pid_t first_pid;
    pid_t second_pid;
    int buffer[2];
    int exit_status;
    
    if(pipe(buffer) < 0 ) {
        error(1, errno, "pipe creation failed");
    }
    first_pid = fork();
    if(first_pid < 0) {
        error(1, errno, "fork failed, pid < 0");
    } else if(first_pid == 0) { //child
        close(buffer[1]); // close unused write end
        if(dup2(buffer[0], 0) < 0) {
            error(1, errno, "dup2 failed");
        } 
        execute_switch(c->u.command[1]);
        _exit(c->u.command[1]->status);
    } else { // pid > 0, so parent
        second_pid = fork();
        if(second_pid < 0) {
            close(buffer[0]); // close unused read end
            if(dup2(buffer[1], 1) < 0) {
                error(1, errno, "dup2 failed");
            }
            execute_switch(c->u.command[0]);
            _exit(c->u.command[0]->status);
        } else { // parent needs overall exit status for pipe command
             returned_pid = waitpid(-1, &exit_status, 0); //-1 means waiting for any process to exit
        }
        // set overall pipe exit status
        if(second_pid == returned_pid) {
            waitpid(first_pid, &exit_status, 0);
            c->status = WEXITSTATUS(estatus);
        } else {
            waitpid(second_pid, &exit_status, 0);
            c->status = WEXITSTATUS(estatus);
        }
    }
}
   

/* switch statement that figures out what kind of command needs to be 
   executed and executes it. */
void
execute_switch(command_t c) {
    switch(c->command_type)
    {
        case SIMPLE_COMMAND:
            // execute_simple_command(c);
            break;
        case IF_COMMAND:
            // execute_if_command(c);
            break;
        case PIPE_COMMAND:
            execute_pipe_command(c);
            break;
        case SEQUENCE_COMMAND:
            // execute_sequence_command(c);
            break;
        case SUBSHELL_COMMAND:
            // execute_subshell_command(c);
            break;
        case UNTIL_COMMAND:
            // execute_until_command(c);
            break;
        case WHILE_COMMAND:
            // execute_while_command(c);
            break;
        default:
            error(1, errno, "invalid command type passed to execute_switch");
            break;
    }
    
}

int
prepare_profiling (char const *name)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
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
  /* FIXME: Replace this with your implementation, like 'prepare_profiling'.  */
  error (1, 0, "command execution not yet implemented");
}
