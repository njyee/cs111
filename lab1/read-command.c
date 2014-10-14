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

#include <stdio.h>
#include <stdlib.h>  // for malloc/free
#include <error.h>
#include <ctype.h>   // for isalnum

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */
   
/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */

#define EXECUTION_STATUS -1;

enum Elements
{
    COMMAND,
    OPERATOR,   // pipe or semicolon
    NEWLINE,
    SPECIAL,    // close parenthesis and special words like "if"
    REDIRECT
};

typedef struct command_node   *command_node_t;
// typedef struct command_stack  command_stack_t;
// typedef struct operator_stack operator_stack_t;

struct command_node
{
    command_t      command; //lots of stuff is named command
    command_node_t next;
};

struct command_stream
{
    command_node_t head;
};

struct command_stack {
    struct command_node *top;
    int size;
};

void init_command_stack(struct command_stack *stack) {
    stack->top = NULL;
    stack->size = 0;
}

// push node to top and increment size
// returns void
void command_stack_push(struct command_stack *stack, struct command_node node) {
    if(stack->size == 0) {
        stack->top = (struct command_node*)malloc(sizeof(struct command_node));
        stack->top->next = NULL;
        stack->top->command = node.command;
        stack->size++;
    } else {
        struct command_node* temp = (struct command_node*)malloc(sizeof(struct command_node));
        temp->command = node.command;
        temp->next = stack->top;
        stack->top = temp;
        stack->size++;
    }
}

// pop the top and decrement size
// returns the popped top
struct command_node command_stack_pop(struct command_stack *stack) {
    struct command_node retval; 
    struct command_node newtop;
    
    retval = *(stack->top); 
    newtop = stack->top->next;
    free(stack->top);
    stack->top = newtop;
    return retval;
}

// peeks the top
// returns a pointer to the top node
struct command_node *command_stack_top(struct command_stack *stack) {
    return stack->top;
}

// checks to see if stack is empty or not
// if empty, returns true else false
int command_stack_empty(struct command_stack *stack) {
    return stack->size == 0;
}

// returns size of command stack
int command_stack_size(struct command_stack *stack) {
    return stack->size;
}

struct operator_node {
    int value;
    struct operator_node* next;
};

struct operator_stack {
    struct operator_node* top;
    int size;
};

void init_operator_node(struct operator_node* node) {
    node->next = NULL;
}

void init_operator_stack (struct operator_stack *stack) {
    stack->top = NULL;
    stack->size = 0;
}

void operator_stack_push( struct operator_stack *stack, struct operator_node node) {
    if(stack->size == 0) {
        stack->top = (struct operator_node*)malloc(sizeof(struct operator_node));
        stack->top->value = node->value;
        stack->top->next = NULL;
        stack->size++;
    } else {
        struct operator_node* temp = (struct operator_node*)malloc(sizeof(struct operator_node));
        temp->value = node->value;;
        temp->next = stack->top;
        stack->top = temp;
        stack->size++;
    }
}

struct operator_stack_pop(struct operator_stack *stack) {
    struct operator_node retval = *(stack->top);
    struct operator_node *temp = stack->top->next;
    free(stack->top);
    stack->top = temp;
    return retval;
}

struct operator_node* operator_stack_top(struct operator_stack *stack) {
    return stack->top;
}

int operator_stack_empty(struct operator_stack *stack) {
    return stack->size == 0;
}

int operator_stack_size(struct operator_stack * stack) {
    return stack->size;
}

// checks if c is an 'other' token as specified in spec
int is_other_character(char c) {
    if(c == '!')
        return 1;
    else if(c == '+')
        return 1;
    else if(c == ',')
        return 1;
    else if(c == '-')
        return 1;
    else if(c == '.')
        return 1;
    else if(c == '/')
        return 1;
    else if(c == ':')
        return 1;
    else if(c == '@')
        return 1;
    else if(c == '^')
        return 1;
    else if(c == '_')
        return 1;
    else 
        return 0;
}

// checks if c is a 'special' token
int is_special_token(char c){
    if(c == ';')
        return 1;
    else if(c == '|')
        return 1;
    else if(c == '(')
        return 1;
    else if(c == ')')
        return 1;
    else if(c == '<')
        return 1;
    else if(c == '>')
        return 1;
    else
        return 0;
}

int is_valid_character(char c) {
    return isalnum(c) || isspace(c) || is_other_token(c) || is_special_token(c);
}

int get_operator_type(char *buf) {
    if(strcmp(buf, "if")) {
        return IF_OP;
    } else if(strcmp(buf, "then")) {
        return THEN_OP;
    } else if(strcmp(buf, "else")) {
        return ELSE_OP;
    } else if(strcmp(buf, "then")) {
        return THEN_OP;
    } else if(strcmp(buf, "fi")) {
        return FI_OP;
    } else if(strcmp(buf, ";")) {
        return SEMICOLON_OP;
    } else if(strcmp(buf, "\n")) {
        return NEWLINE_OP;
    } else if(strcmp(buf, "|")) {
        return PIPE_OP;
    } else
        return -1; // error: not an operator
}

// based on operator, return appropriate command type
int get_command_type(int op) {
    switch(op) {
        case IF_OP:
        case THEN_OP:
        case ELSE_OP:
        case FI_OP:
            return IF_COMMAND;
            break;
        case OPEN_PAREN_OP:
        case CLOSE_PAREN_OP:
            return SUBSHELL_COMMAND;
            break;
        case SEMICOLON_OP:
        case NEWLINE_OP:
            return SEQUENCE_COMMAND;
            break;
        case PIPE_OP:
            return PIPE_COMMAND;
            break;
        default:
            //error
            return -1;
    }
}

// used to combine 2 command nodes into a single command node
struct command_node combine_commands(struct command_node first_command, struct command_node second_command, int op) {
    struct command_node new_command_node; 
    
    // get command type from the operator type
    int command_type = get_command_type(op);
    
    // allocate memory for commands to be stored within new_command
    command_t first  = (command_t)malloc(sizeof(struct command));
    command_t second = (command_t)malloc(sizeof(struct command));
    
    *first  = *(first_command.command);
    *second = *(second_command.command);
    
    new_command_node.type    = command_type;
    new_command_node.status  = EXECUTION_STATUS;
    new_command_node.input   = NULL;
    new_command_node.output  = NULL;
    
    // place objects in u.command
    new_command_node->command->u.command[0] = first;
    new_command_node->command->u.command[1] = second;
    
    return new_command_node;
}

// overload of combine_commands that takes 3 command nodes
// for use with if-then-lse commands
struct command_node combine_commands(struct command_node first_command, struct command_node second_command, struct command_node third_command, int op) {
    struct command_node new_command_node; 
    
    // get command type from the operator type
    int command_type = get_command_type(op); // op == IF_OP
    
    // allocate memory for commands to be stored within new_command
    command_t first  = (command_t)malloc(sizeof(struct command));
    command_t second = (command_t)malloc(sizeof(struct command));
    command_t third  = (command_t)malloc(sizeof(struct command));
    
    *first  = *(first_command.command);
    *second = *(second_command.command);
    *third  = *(third_command.command);
    
    // fill in command attributes
    new_command_node.type    = command_type;
    new_command_node.status  = EXECUTION_STATUS;
    new_command_node.input   = NULL;
    new_command_node.output  = NULL;
    
    // place objects in u.command
    new_command_node->command->u.command[0] = first;
    new_command_node->command->u.command[1] = second;
    new_command_node->command->u.command[2] = third;
    
    return new_command_node;
}


command_stream_t
make_command_stream (int (*get_next_byte) (void *),
                 void *get_next_byte_argument)
{
    /* FIXME: Replace this with your implementation.  You may need to
       add auxiliary functions and otherwise modify the source code.
       You can also use external functions defined in the GNU C Library.  */

    int j;
    size_t i;

    int c = 0;
    int is_operator = 0;
    int last_byte = 0;
    int number_of_words = 0;
    
    char word[100];
    char* words[100];
    
    enum Elements follows;
    
    command_node_t node;
    
    struct operator_stack opstack;
    struct command_stack comstack;
    
    struct command_stream comstream;
    
    init_operator_stack(&opstack);
    init_command_stack(&comstack);

    for (;;)
    {
        // Reset
        is_operator = 0;

        // get next byte
        c = get_next_byte(get_next_byte_argument);
        
        // if word char
        if (isalnum(c) || c == '!' || c == '%' || c == '+' || c == ',' || c == '-' ||
              c == '.' || c == '/' || c == ':' || c == '@' || c == '^' || c == '_' )
        {
            // if first char of first word and follows command
            if (strlen(word) == 0 && sizeof(words) == 0 && follows == COMMAND)
            {
                // error
                ;
            }
            // append (could have huge problems with segfault)
            i = strlen(word);
            word[i] = c;
            word[++i] = '\0';
        }
        else
        {
            if (strlen(word) > 0)
            {
                // if first word
                if (sizeof(words) == 0)
                {
                    if (strcmp(word, "if") || strcmp(word, "until"))
                    {
                        // push onto operator stack
                    }
                    else if (strcmp(word, "then") || strcmp(word, "else") || strcmp(word, "if") ||
                             strcmp(word, "do") || strcmp(word, "done"))
                    {
                        // validate (scope stack?)
                        // process operator stack until reach preceding special word
                    }
                }
                words[number_of_words++] = word;
                word = (char*) malloc(sizeof(word));
                word[0] = '\0';
            }
            if (c == '#')
            {
                // ignore remaining characters until reach newline
                for (;;) {
                    c = get_next_byte(get_next_byte_argument);
                    if (c == '\n' || c == EOF)
                        break;
                }
            }
            if (sizeof(words) > 0) // then simple command
            {
                // push simple command
                node->command->type = SIMPLE_COMMAND;
                node->command->status = EXECUTION_STATUS;
                node->command->input = NULL;
                node->command->output = NULL;
                node->command->word = words;
                // node->command->u.command[0] = NULL;
                command_stack_push(&comstack, node);
                
                // reset
                node = (command_node_t) malloc(sizeof(command_node));
                words = (char**) malloc(sizeof(words));
                number_of_words = 0;
            }
            if (c == ';' || c == '|')
            {
                if (follows == COMMAND)
                {
                    is_operator = 1;
                }
                else {
                    // error
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
                // get operator type from operator string
                char *operator_string = &c;
                int operator_type = get_operator_type(operator_string);  //operator_string is made up
                struct operator_node op_node;
                
                op_node.value = operator_type;
                
                if(operator_stack_empty(&opstack)) {
                    operator_stack_push(&opstack, op_node);
                } else if(operator_type > operator_stack_top(&opstack).value 
                            || operator_type == IF_OP) {
                    operator_stack_push(&opstack, op_node);
                } else {
                    struct operator_node *opstack_top = operator_stack_top(&opstack);
                    while ( (opstack_top->value != OPEN_PAREN_OP || opstack_top->value != THEN_OP
                            || opstack_top->value != ELSE_OP || opstack_top->value != IF_OP )
                            && operator_type <= opstack_top->value) {
                                struct operator_node popped_operator = operator_stack_pop(&opstack);
                                struct command_node second_command   = command_stack_pop(&comstack);
                                struct command_node first_command    = command_stack_pop(&comstack);
                                struct command_node new_command = combine_commands(first_command, second_command, popped_operator);
                                
                                opstack_top = operator_stack_top(&opstack);
                                if(opstack_top == NULL)
                                    break;
                            }
                            operator_stack_push(&opstack, op_node);
                            if(operator_stack_top(&opstack)->value == FI_OP) {
                                operator_stack_pop(&opstack);  // don't need FI_OP
                                opstack_top = operator_stack_pop(&opstack);
                                if(opstack_top == ELSE_OP) {
                                    struct command_node third_command  = command_stack_pop(&comstack);
                                    struct command_node second_command = command_stack_pop(&comstack);
                                    struct command_node first_command  = command_stack_pop(&comstack);
                                    struct command_node new_command    = combine_commands(first_command, second_command, third_command, ELSE_OP);
                                    command_stack_push(new_command);
                                    operator_stack_pop(&opstack); //should be popping THEN_OP
                                    operator_stack_pop(&opstack); //should be popping IF_OP
                                } else if(opstack_top == THEN_OP) {
                                    struct command_node second_command = command_stack_pop(&comstack);
                                    struct command_node first_command  = command_stack_pop(&comstack);
                                    struct command_node new_command    = combine_commands(first_command, second_command, IF_OP);
                                    operator_stack_pop(&opstack); //should be popping IF_OP
                                }
                                
                            }
                }
                
                
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
            }
        }

        last_byte = c;
    }

    comstream.head = comstack.top;
    
    return &comstream;
}

command_t
read_command_stream (command_stream_t s)
{
    /* FIXME: Replace this with your implementation too.  */
    
    command_t c = NULL;
    if (s->head != NULL) {
        c = s->head->command;
        s->head = s->head->next;
    }
    
    return c;
}
