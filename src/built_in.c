/******************************************************************************
# file: /home/TU/tug35668/lab/cis_3207/lab_2/built_in.c
#
# Tarek Elseify
# February 4, 2019
# Lab 2 - Developing a Linux Shell
# CIS 3207 SEC 001
# tug35668@temple.edu
#
# This program contains the builtin methods of the shell
******************************************************************************/

/* standard include statements */
/*                             */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h> 
#include <unistd.h>
#include <dirent.h>
#include "shell.h"

/* environment variables */
/*                       */
extern char **environ;


/******************************************************************************
#   function: in_cd
#
#   arguments: command *cmd - pointer to command
#
#   returns: int (1 or 0) depending on success/failure
#
#   this function changes the users working directory
******************************************************************************/
int in_cd(command *cmd)
{

  /* get the current command (should only be one) */
  /*                                              */
  command *current = cmd;

  /* placeholder for the directory to change into */
  /*                                              */
  char *dir;
  
  /* set the directory from the options */
  /*                                    */
  dir = current->options[0];

  /* if there is no directory provided */
  /*                                   */
  if(dir == NULL){

    /* try getting the home directory */
    /*                                */
    if((dir = getenv("HOME")) == NULL){

      /* print an error */
      /*                */
      printf("%s\n", "shell: could not get home directory");
      return 0;
    }
  }
  
  /* change the directory */
  /*                      */
  int worked = chdir(dir);

  /* print an error if this has failed */
  /*                                   */
  if(worked != 0){
    fprintf(stderr, "%s\n", "shell: invalid directory");
    return 0;
  }

  /* get the current working directory and print */
  /*                                             */
  char cwd[PATH_MAX];
  if(getcwd(cwd, sizeof(cwd)) != NULL){
    printf("%s\n", cwd);
  }
  
  /* exit gracefully */
  /*                 */
  return 1;
}
/*                 */
/* end of function */


/******************************************************************************
#   function: in_clr
#
#   arguments: command *cmd - pointer to command
#
#   returns: int (1 or 0) depending on success/failure
#
#   this function clears the user's screen
******************************************************************************/
int in_clr(command *cmd)
{
  /* return if other options have been set */
  /*                                       */
  if((cmd->options[0] != NULL) || (cmd->arg_list[0] != NULL)){
    fprintf(stderr, "%s\n", "shell: unknown options");
    return 0;
  }

  /* clear the screen */
  /*                  */
  printf("\033[H\033[2J");

  /* exit gracefully */
  /*                 */
  return 1;
}
/*                 */
/* end of function */


/******************************************************************************
#   function: in_dir
#
#   arguments: command *cmd - pointer to command
#
#   returns: int (1 or 0) depending on success/failure
#
#   this function displays the content of a given directory
******************************************************************************/
int in_dir(command *cmd)
{

  /* try to get the directory */
  /*                          */
  char *dir = (char *) malloc(sizeof(char) * PATH_MAX);

  /* try getting the current working directory */
  /*                                           */
  char cwd[PATH_MAX];

  /* set the directory to cwd if none is given */
  /*                                           */
  if(cmd->options[0] != NULL){
    dir = cmd->options[0];
  }else{

    /* try getting the cwd */
    /*                     */
    if(getcwd(cwd, sizeof(cwd)) == NULL){
      fprintf(stderr, "%s\n", "shell: failed to get cwd");
      return 0;
    }

    /* set dir to the cwd */
    /*                    */
    strcpy(dir, cwd);
  }
  
  /* stream of dir will be here */
  /*                            */
  DIR *content;
  struct dirent *stream;
  
  /* if we could not open the directory, return */
  /*                                            */
  if((content = opendir(dir)) == NULL){
    fprintf(stderr, "%s\n", "shell: invalid directory");
    return 0;
  }

  /* print while we haven't run out of things inside the directory */
  /*                                                               */
  while((stream = readdir(content)) != NULL){
    printf("%s\n", stream->d_name);
  }

  /* close the directory */
  /*                     */
  closedir(content);

  /* exit gracefully */
  /*                 */
  return 1;
}
/*                 */
/* end of function */


/******************************************************************************
#   function: in_environ
#
#   arguments: command *cmd - pointer to command
#
#   returns: int (1 or 0) depending on success/failure
#
#   this function displays the environment variables
******************************************************************************/
int in_environ(command *cmd)
{

  /* there should be no options/arguments */
  /*                                      */
  if((cmd->options[0] != NULL) || (cmd->arg_list[0] != NULL)){
    fprintf(stderr, "%s\n", "shell: unknown options");
    return 0;
  }
  
  /* index count */
  /*             */
  int index = 1;

  /* current variable */
  /*                  */
  char *current = *environ;

  /* for all the variables */
  /*                       */
  for(; current; index++){

    /* print the environment variable */
    /*                                */
    printf("%s\n", current);

    /* increment current pointer */
    /*                           */
    current = *(environ+index);
  }

  /* exit gracefully */
  /*                 */
  return 1;
}
/*                 */
/* end of function */


/******************************************************************************
#   function: in_echo
#
#   arguments: command *cmd - pointer to command
#
#   returns: int (1 or 0) depending on success/failure
#
#   this function prints the user argument
******************************************************************************/
int in_echo(command *cmd)
{

  /* argument index */
  /*                */
  int index = -1;

  /* while the argument is not null */
  /*                                */
  while(cmd->options[++index] != NULL){
    printf("%s ", cmd->options[index]);
  }

  /* enter a new line */
  /*                  */
  printf("%c", '\n'); 

  /* exit gracefully */
  /*                 */
  return 1;
}
/*                 */
/* end of function */


/******************************************************************************
#   function: in_help
#
#   arguments: command *cmd - pointer to command
#
#   returns: int (1 or 0) depending on success/failure
#
#   this function prints the user manual
******************************************************************************/
int in_help(command *cmd)
{

  /* there should be no options or arguments */
  /*                                         */
  if((cmd->options[0] != NULL) || (cmd->arg_list[0] != NULL)){
    fprintf(stderr, "%s\n", "shell: unknown options");
    return 0;
  }

  /* set the argv */
  /*              */
  char *argv[] = {"more", "readme.txt", NULL};

  /* hold the child pid */
  /*                    */
  pid_t child_pid;
  
  /* fork the execution */
  /*                    */
  if((child_pid = fork()) == -1){
    fprintf(stderr, "%s\n", "shell: failed to create a fork");
    return 0;
  }

  /* if this is the child process, execute the command */
  /*                                                   */
  if(child_pid == 0){
    if(execvp(argv[0], argv) < 0){
      fprintf(stderr, "%s\n", "shell: failed to execute more command");
      return 0;
    }
  }

  /* wait for the child to complete */
  /*                                */
  wait(NULL);
  
  /* exit gracefully */
  /*                 */
  return 1; 
}
/*                 */
/* end of function */


/******************************************************************************
#   function: in_pause
#
#   arguments: command *cmd - pointer to command
#
#   returns: int (1 or 0) depending on success/failure
#
#   this function pauses the shell until newline is entered
******************************************************************************/
int in_pause(command *cmd)
{

  /* there should be no options or arguments */
  /*                                         */
  if((cmd->options[0] != NULL) || (cmd->arg_list[0] != NULL)){
    fprintf(stderr, "%s\n", "shell: unknown options");
    return 0;
  }

  /* inform the user that they need to press enter to continue */
  /*                                                           */
  printf("%s\n", "shell: press ENTER to continue");

  /* while the char entered is not newl */
  /*                                    */
  while(getchar() != '\n');

  /* exit gracefully */
  /*                 */
  return 1;
}
/*                 */
/* end of function */


/******************************************************************************
#   function: in_exit
#
#   arguments: command *cmd - pointer to command
#
#   returns: int (1 or 0) depending on success/failure
#
#   this function exits the shell
******************************************************************************/
int in_exit(command *cmd)
{


  /* there should be no options or arguments */
  /*                                         */
  if((cmd->options[0] != NULL) || (cmd->arg_list[0] != NULL)){
    fprintf(stderr, "%s\n", "shell: unknown options");
    return 0;
  }

  /* exit the shell */
  /*                */
  exit(0);

  /* unreachable */
  /*             */
  return 0;
}
/*                 */
/* end of function */


/*****************************************************************************/
/*---------------------------------end of file-------------------------------*/

