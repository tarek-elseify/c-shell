/******************************************************************************
# file: /home/TU/tug35668/lab/cis_3207/lab_2/shell.h
#
# Tarek Elseify
# February 4, 2019
# Lab 2 - Developing a Linux Shell
# CIS 3207 SEC 001
# tug35668@temple.edu
#
# This program contains the necessary struct and function protypes
# for the main program to use
******************************************************************************/

/* standard include statements */
/*                             */
#ifndef SHELL_H_
#define SHELL_H_
#include <stddef.h>

/* define vars to be used throughout the program */
/*                                               */
#define SHELL_PROMPT "tarek: "
#define DELIM_WHITESPACE " \t\n"
#define DELIM_OPTION '-'
#define DELIM_BCKG '&'
#define DELIM_PIPE '|'
#define DELIM_REDIR_STDOUT '>'
#define DELIM_REDIR_STDIN '<'

/* struct to define a command */
/*                            */
typedef struct
{
  char *name;
  char **options;
  char **arg_list;
  int CASE;
}command;

/* enumeration to distinguish different options */
/*                                              */
enum OPTIONS{NORMAL, STDIN, STDOUT, STDOUT_APPEND, PIPE, BCKG}; 

/* built in commands prototype */
/*                             */
int in_cd(command *cmd);
int in_clr(command *cmd);
int in_dir(command *cmd);
int in_environ(command *cmd);
int in_echo(command *cmd);
int in_help(command *cmd);
int in_pause(command *cmd);
int in_exit(command *cmd);
int path(command *cmd);
int execute_command(command *cmd, int fork_p);

/* define the PATH */
/*                 */
extern char *PATH[100];
extern int NUM_PATHS;

#endif // SHELL_H_

/*****************************************************************************/
/*---------------------------------end of file-------------------------------*/
