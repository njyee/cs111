// UCLA CS 111 Lab 1 command reading

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

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

enum Elements
{
    COMMAND,
    OPERATOR,   // pipe or semicolon
    NEWLINE,
    SPECIAL,    // close parenthesis and special words like "if"
    REDIRECT
};

typedef struct command_node   *command_node_t;
// typedef struct command_stack  *command_stack_t;
// typedef struct operator_stack *operator_stack_t;

struct command_node
{
    command_t      command;
    command_node_t next;
}

/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */

struct command_stream
{
    command_node_t head;
}

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
                 void *get_next_byte_argument)
{
    /* FIXME: Replace this with your implementation.  You may need to
       add auxiliary functions and otherwise modify the source code.
       You can also use external functions defined in the GNU C Library.  */

    int c, is_operator, last_byte;
    char word[100];
    char* words[100];
    enum Elements follows;

    for (;;)
    {
        // Reset
        word[0] = 0;
        words[0] = NULL;
        is_operator = 0;

        // get next byte
        c = get_next_byte(get_next_byte_argument);
        
        // if word char
        if (('A' <= c && c <= 'Z') || ('a' <= c && c<= 'Z') ||
            c == '!' || c == '%' || c == '+' || c == ',' || c == '-' || c == '.'
            c == '/' || c == ':' || c == '@' || c == '^' || c == '_' )
        {
            // if first char of first word and follows command
            if (strlen(word) == 0 && sizeof(words) == 0 && follows == COMMAND)
            {
                // error
                ;
            }
            // append
        }
        // else
        else
        {
            if (strlen(word) > 0)
            {
                // end current word
                ;
                // if first word
                if (sizeof(words) == 0)
                {
                    if (strcmp(word, "if") || strcmp(word, "until"))
                    {
                        // push onto operator stack
                        ;
                    }
                    else if (strcmp(word, "then") || strcmp(word, "else") || strcmp(word, "if") ||
                             strcmp(word, "do") || strcmp(word, "done"))
                    {
                        // validate (scope stack?)
                        // process operator stack until reach preceding special word
                    }
                }
            }
            else if (c == '#')
            {
                // ignore remaining characters until reach newline
                ;
            }
            else
            {
                if (sizeof(words) > 0)  // then simple command
                {
                    // push simple command
                }
                if (c == ';' || c == '|')
                {
                    if (follows == COMMAND)
                    {
                        is_operator = 1;
                    }
                    else {
                        // error
                        ;
                    }
                }
                else if (c == '\n')
                {
                    if (follows == COMMAND)
                    {
                        c = ';';
                        is_operator = 1;
                    }
                    else if (last_byte == ';')
                    {
                        follows = NEWLINE;
                    }
                }
                // else if < or >
                    // if follows command
                        // if simple command
                            // push
                        // else
                            // modify last command
                    // else
                        // error
                if (is_operator)
                {
                    // algorithm
                }
            }
        }

        last_byte = c;
    }

    // special operator cases:
    // consecutive \n or ;
    // ; not followed by command
    // <> not followed by word, followed by multiple words

    // Parse algorithm

    // if encounter new commands (simple command)
    //     put them on command stack
    // if encounter new operator
    //     if operator_stack == NULL
    //         Add new operator to operator stack
    //     else if precedence(new operator) > precedence(opstack.top) || new operator == 'if' 
    //         Add new oparator to operator stack
    //     else
    //         while top.operator != (OPEN.PARENTHESIS || 'then' || 'else' || 'if') && precedence(new operator) <= precedence(top operator)
    //           {
    //             operator = operator-stack.pop()
    //             second-command = command-stack.pop()
    //             first-command = command-stack.pop()
    //             new-command = combine(first-command, second-command, operator)
    //             command-stack.push(new-command)
    //             top-operator = operator-stack.peek()

    //             if top-operator == NULL
    //                 break;
    //           }
    //         operator-stack.push(new_operator)
    //         if(operator-stack.top == "fi")
    //             operator-stack.pop()
    //             if(operator-stack.top == "then")
    //                 pop and process twice
    //             else if(operator-stack.top == "else")
    //                 pop and process 3 times

    return 0;
}

command_t
read_command_stream (command_stream_t s)
{
    /* FIXME: Replace this with your implementation too.  */
    error (1, 0, "command reading not yet implemented");
    return 0;
}
