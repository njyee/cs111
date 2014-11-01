// UCLA CS 111 Lab 1 main program

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

#include <errno.h>
#include <error.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <sys/resource.h>
#include <math.h>
#include <string.h>
#include <fcntl.h>

#include "command.h"

static char const *program_name;
static char const *script_name;

static void
usage (void)
{
  error (1, 0, "usage: %s [-p PROF-FILE | -t] SCRIPT-FILE", program_name);
}

static int
get_next_byte (void *stream)
{
  return getc (stream);
}

int
main (int argc, char **argv)
{
  int command_number = 1;
  bool print_tree = false;
  char const *profile_name = 0;
  
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
  
  p = open("log", O_CREAT | O_WRONLY | O_APPEND, 0644);
  
  program_name = argv[0];

  for (;;)
    switch (getopt (argc, argv, "p:t"))
      {
      case 'p': profile_name = optarg; break;
      case 't': print_tree = true; break;
      default: usage (); break;
      case -1: goto options_exhausted;
      }
 options_exhausted:;

  // There must be exactly one file argument.
  if (optind != argc - 1)
    usage ();

  script_name = argv[optind];
  FILE *script_stream = fopen (script_name, "r");
  if (! script_stream)
    error (1, errno, "%s: cannot open", script_name);
  command_stream_t command_stream =
    make_command_stream (get_next_byte, script_stream);
  int profiling = -1;
  if (profile_name)
    {
      profiling = prepare_profiling (profile_name);
      if (profiling < 0)
	error (1, errno, "%s: cannot open", profile_name);
    }

  command_t last_command = NULL;
  command_t command;
  while ((command = read_command_stream (command_stream)))
    {
      if (print_tree)
	{
	  printf ("# %d\n", command_number++);
	  print_command (command);
	}
      else
	{
	  last_command = command;
	  execute_command (command, profiling);
	}
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
  snprintf(tmp, BYTE_LIMIT, "[%d]", p);

  strncat(buf, tmp, BYTE_LIMIT - strlen(buf) - 1);

  write(p, (const void *) buf, strlen(buf));
  write(p, (const void *) newline, 1);

  return print_tree || !last_command ? 0 : command_status (last_command);
}
