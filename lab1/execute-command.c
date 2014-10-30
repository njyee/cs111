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
#include <time.h>
#include <sys/resource.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#define BYTE_LIMIT 1024
// #define ARBITRARY_LIMIT 100


void set_io(command_t c) {
    int infile_desc;
    int outfile_desc;

    if(c->input != NULL) {
        infile_desc = open(c->input, O_RDONLY);
        dup2(infile_desc, 0);
    }
    if(c->output != NULL) {
        //outfile_desc = open(c->output, O_WRONLY | O_APPEND | O_CREAT, 0644);
        outfile_desc = open(c->output, O_WRONLY | O_CREAT, 0644);
        dup2(outfile_desc, 1);
    }
}

/*void propagate_io(command_t c) {
    int i = 0;
    if(c->input != NULL)
        while(i < 3 && c->u.command[i] != NULL) {
            if (c->u.command[i]->input == NULL)
                c->u.command[i]->input = c->input;
            i++;
        }
    i = 0;
    if(c->output != NULL)
        while(i < 3 && c->u.command[i] != NULL) {
            if (c->u.command[i]->output == NULL)
                c->u.command[i]->output = c->output;
            i++;
        }
}*/
   
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
    
    // propagate_io(c);
    set_io(c);
    
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
    
    // propagate_io(c);
    set_io(c);

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
    
    // propagate_io(c);
    set_io(c);

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


static void
print_command_prof (int indent, command_t c, char *buf);

/* switch statement that figures out what kind of command needs to be 
   executed and executes it. */
void
execute_switch(command_t c) {
    int p;

    struct timespec start, end, absolute;
    struct rusage self, children;

    double real_time, absolute_time, user_usage, system_usage;

    char buf[BYTE_LIMIT];
    char tmp[BYTE_LIMIT];
    char newline[] = "\n";
    memset(buf, 0, BYTE_LIMIT);
    memset(tmp, 0, BYTE_LIMIT);

    clock_gettime(CLOCK_MONOTONIC, &start);

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

    clock_gettime(CLOCK_MONOTONIC, &end);
    clock_gettime(CLOCK_REALTIME, &absolute);

    getrusage(RUSAGE_SELF, &self);
    getrusage(RUSAGE_CHILDREN, &children);

    absolute_time = (double) absolute.tv_sec + (double) (absolute.tv_nsec * pow(10, -9));
    real_time = (double) end.tv_sec
              - (double) start.tv_sec
              + (double) (end.tv_nsec * pow(10, -9))
              - (double) (start.tv_nsec * pow(10, -9));

    user_usage = (double) self.ru_utime.tv_sec + (double) (self.ru_utime.tv_usec * pow(10, -6))
               + (double) children.ru_utime.tv_sec + (double) (children.ru_utime.tv_usec * pow(10, -6));

    system_usage = (double) self.ru_stime.tv_sec + (double) (self.ru_stime.tv_usec * pow(10, -6))
                 + (double) children.ru_stime.tv_sec + (double) (children.ru_stime.tv_usec * pow(10, -6));

    // format time and usage to correct precision

    snprintf(tmp, BYTE_LIMIT, "%f ", absolute_time);
    strncat(buf, tmp, BYTE_LIMIT - strlen(buf) - 1);
    snprintf(tmp, BYTE_LIMIT, "%f ", real_time);
    strncat(buf, tmp, BYTE_LIMIT - strlen(buf) - 1);
    snprintf(tmp, BYTE_LIMIT, "%f ", user_usage);
    strncat(buf, tmp, BYTE_LIMIT - strlen(buf) - 1);
    snprintf(tmp, BYTE_LIMIT, "%f ", system_usage);
    strncat(buf, tmp, BYTE_LIMIT - strlen(buf) - 1);

    memset(tmp, 0, BYTE_LIMIT);
    print_command_prof(0, c, tmp);

    strncat(buf, tmp, BYTE_LIMIT - strlen(buf) - 1);

    p = open("log", O_CREAT | O_WRONLY, 0644);
    write(p, (const void *) buf, strlen(buf));
    write(p, (const void *) newline, 1);
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
//    pid_t p;
//    int exit_status;

//    p = fork();
//    if(p<0)
//        error(1, errno, "fork failed");
//    else if(p == 0) {
        execute_switch(c);
//        _exit(c->status);
//    }
//    else
//        waitpid(p, &exit_status, 0);
}


static void
print_command_prof (int indent, command_t c, char *buf)
{
  char tmp[BYTE_LIMIT];
  memset(tmp, 0, BYTE_LIMIT);
  indent = 0;
  switch (c->type)
    {
    case IF_COMMAND:
    case UNTIL_COMMAND:
    case WHILE_COMMAND:
      snprintf (tmp, BYTE_LIMIT, "%*s%s ", indent, "",
          (c->type == IF_COMMAND ? "if"
           : c->type == UNTIL_COMMAND ? "until" : "while"));
      strncat(buf, tmp, BYTE_LIMIT - strlen(buf) - 1);
      print_command_prof (indent + 2, c->u.command[0], buf);
      snprintf (tmp, BYTE_LIMIT, " %*s%s ", indent, "", c->type == IF_COMMAND ? "then" : "do");
      strncat(buf, tmp, BYTE_LIMIT - strlen(buf) - 1);
      print_command_prof (indent + 2, c->u.command[1], buf);
      if (c->type == IF_COMMAND && c->u.command[2])
    {
      snprintf (tmp, BYTE_LIMIT, " %*selse ", indent, "");
      strncat(buf, tmp, BYTE_LIMIT - strlen(buf) - 1);
      print_command_prof (indent + 2, c->u.command[2], buf);
    }
      snprintf (tmp, BYTE_LIMIT, " %*s%s", indent, "", c->type == IF_COMMAND ? "fi" : "done");
      strncat(buf, tmp, BYTE_LIMIT - strlen(buf) - 1);
      break;

    case SEQUENCE_COMMAND:
    case PIPE_COMMAND:
      {
    print_command_prof (indent + 2 * (c->u.command[0]->type != c->type),
                c->u.command[0], buf);
    char separator = c->type == SEQUENCE_COMMAND ? ';' : '|';
    snprintf (tmp, BYTE_LIMIT, " %*s%c ", indent, "", separator);
    strncat(buf, tmp, BYTE_LIMIT - strlen(buf) - 1);
    print_command_prof (indent + 2 * (c->u.command[1]->type != c->type),
                c->u.command[1], buf);
    break;
      }

    case SIMPLE_COMMAND:
      {
    char **w = c->u.word;
    snprintf (tmp, BYTE_LIMIT, "%*s%s", indent, "", *w);
    strncat(buf, tmp, BYTE_LIMIT - strlen(buf) - 1);
    while (*++w)
      snprintf (tmp, BYTE_LIMIT, " %s", *w);
      strncat(buf, tmp, BYTE_LIMIT - strlen(buf) - 1);
    break;
      }

    case SUBSHELL_COMMAND:
      snprintf (tmp, BYTE_LIMIT, "%*s( ", indent, "");
      strncat(buf, tmp, BYTE_LIMIT - strlen(buf) - 1);
      print_command_prof (indent + 1, c->u.command[0], buf);
      snprintf (tmp, BYTE_LIMIT, " %*s)", indent, "");
      strncat(buf, tmp, BYTE_LIMIT - strlen(buf) - 1);
      break;

    default:
      abort ();
    }

  if (c->input) {
    snprintf (tmp, BYTE_LIMIT, "<%s", c->input);
    strncat(buf, tmp, BYTE_LIMIT - strlen(buf) - 1);
  }
  if (c->output) {
    snprintf (tmp, BYTE_LIMIT, ">%s", c->output);
    strncat(buf, tmp, BYTE_LIMIT - strlen(buf) - 1);
  }
}
