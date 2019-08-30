/******************************************************************************
# file: /home/TU/tug35668/lab/cis_3207/lab_2/shell.c
#
# Tarek Elseify
# February 4, 2019
# Lab 2 - Developing a Linux Shell
# CIS 3207 SEC 001
# tug35668@temple.edu
#
# This program creates a simple Linux/Unix shell.
******************************************************************************/

/* standard include statements */
/*                             */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "shell.h"

/* funtion prototypes are listed here */
/*                                    */
char **parse_cmdline(char *commands);
command **arg_parser(char **commands);
int execute_command(command *cmd, int fork_p);
int is_option(char *command);
int set_redir_case(command *cmd, char *option);
int process_args(command **cmds);
int pipe_process(command **cmds);
int redirect(command *cmd);
void exec_batch(char *file);

/* store all the internal commands */
/*                                 */
char *internal_cmds[] = {
  "cd",
  "clr",
  "dir",
  "environ",
  "echo",
  "help",
  "pause",
  "exit",
  NULL
};

/* array of pointers to the built in functions */
/*                                             */
int(*built_in[]) (command *) = {
  &in_cd,
  &in_clr,
  &in_dir,
  &in_environ,
  &in_echo,
  &in_help,
  &in_pause,
  &in_exit,
  NULL
};


/******************************************************************************
#   function: main
#
#   arguments: none
#
#   returns: none
#
#   this is the main function of the program
******************************************************************************/
int main(int argc, char *argv[])
{
  
  /* if the user provided more than 2 arguments */
  /*                                            */
  if(argc > 2){

    /* inform the user that there can only be one argument */
    /*                                                     */
    printf("%s\n", " .... only one argument can be passed....");
    exit(0);
  }else if(argc == 2){

    /* execute the commands in batch mode */
    /*                                    */
    exec_batch(argv[1]);
    
    /* if there is a file given to execute */
    /*                                     */    
    exit(0);
  }

  /* vars to be used to read in commands */
  /*                                     */
  size_t max_buff_size = 500;
  char *current_command = NULL;
  char **parsed_cmds;
  command **parsed_args;
  
  /* print the shell prompt */
  /*                        */
  printf("%s", SHELL_PROMPT);

  /* flush stdout */
  /*              */
  fflush(stdout);

  /* infinite loop, always calling getline */
  /*                                       */
  while(getline(&current_command, &max_buff_size, stdin) > -1){

    /* flush stdin */
    /*             */
    fflush(stdin);
    
    /* pointer to each command given */
    /*                               */
    parsed_cmds = parse_cmdline(current_command);

    /* pointer to struct of every commnd */
    /*                                   */
    parsed_args = arg_parser(parsed_cmds);

    /* process and execute the commands */
    /*                                  */
    process_args(parsed_args);

    /* print the shell prompt */
    /*                        */
    printf("%s", SHELL_PROMPT);

    /* flush stdout */
    /*              */
    fflush(stdout);
    
    /* clear the current command to be used again */
    /*                                            */
    free(current_command);
    free(parsed_args);
    current_command = NULL;
    parsed_args = NULL;
  }
  
  /* exit gracefully */
  /*                 */
  return 0;
}
/*                 */
/* end of function */


/******************************************************************************
#   function: execute_command
#
#   arguments: command *cmd - pointer to command struct
#              int fork_p - boolean representing whether or not to fork
#
#   returns: (1 or 0) logical value if executed properly
#
#   this function creates a fork and runs a child process
******************************************************************************/
int execute_command(command *cmd, int fork_p)
{

  /* if the command is null */
  /*                        */
  if(cmd == NULL){
    fprintf(stderr, "%s\n", "shell: could not execute command");
    return 0;
  }
  
  /* int to track cmds */
  /*                   */
  int index = -1;

  /* while we haven't reached the end of the builtin cmds */
  /*                                                      */
  while(internal_cmds[++index] != NULL){

    /* if this is a built in command */
    /*                               */
    if(strcmp(cmd->name, internal_cmds[index]) == 0){

      /* execute the function */
      /*                      */
      return (*built_in[index])(cmd);
    }
  }  

  /* will be used to call execvp */
  /*                             */
  char *argv[50];

  /* indices to be used for argv and cmd */
  /*                                     */
  int arg_i = -1;
  int cmd_i = -1;

  /* used to check status of child process */
  /*                                       */
  int status;

  /* try to find the proper path to the executable */
  /*                                               */
  char *name = cmd->name;

  /* allocate an excess string */
  /*                           */
  char *temp = (char *) malloc(10000);

  /* copy into temp */
  /*                */
  strcpy(temp, name);

  /* set the command name */
  /*                      */
  argv[++arg_i] = temp;

  /* set the options */
  /*                 */
  while(cmd->options[++cmd_i] != NULL){
    argv[++arg_i] = cmd->options[cmd_i];
  }

  /* null terminate argv */
  /*                     */
  argv[++arg_i] = NULL;

  /* if we are not forking, execute the program */
  /*                                            */
  if(!fork_p){
    if(execvp(argv[0], argv) < 0){
      fprintf(stderr, "%s\n", "shell: unknown command");
      exit(0);
    }
  }
  
  /* hold the new process id */
  /*                         */
  pid_t new_pid, wait_pid;

  /* if the fork fails */
  /*                   */
  if((new_pid = fork()) == -1){
    fprintf(stderr, "%s\n", "shell: failed to create fork");
    return 0;
  }

  /* if this is the child process */
  /*                              */
  if(new_pid == 0){    

    /* if we failed to launch the process */
    /*                                    */
    if(execvp(argv[0], argv) < 0){

      /* print error and return */
      /*                        */
      fprintf(stderr, "%s\n", "shell: unknown command");
      exit(0);
    }
  }

  /* if this is a parent process */
  /*                             */
  else{

    /* if the command is not a bckg command */
    /*                                      */
    if(cmd->CASE != BCKG){
      wait_pid = waitpid(new_pid, &status, WUNTRACED); 
      while(!WIFEXITED(status) && !WIFSIGNALED(status)){
	wait_pid = waitpid(new_pid, &status, WUNTRACED);
      }
    }
  }
  
  /* return sucessfully */
  /*                    */
  return 1;
}
/*                 */
/* end of function */


/******************************************************************************
#   function: process_args
#
#   arguments: command **cmds - pointer to command structs
#
#   returns: int (1 or 0) depending on success/failure
#
#   this function process a commands properly depending on redirection
#   pipe, or normal/bckg
******************************************************************************/
int process_args(command **cmds)
{

  /* do nothing if there are no commands given */
  /*                                           */
  if(cmds == NULL){
    return 1;
  }

  /* if this is either normal */
  /*                          */
  if(cmds[0]->CASE == NORMAL){

    /* execute the command */
    /*                     */
    return execute_command(cmds[0], 1);
  }

  /* if this is a background */
  /*                         */
  else if(cmds[0]->CASE == BCKG){

    /* keep track of the commands */
    /*                            */
    int index = -1;

    /* while we haven't run out of commands */
    /*                                      */
    while((cmds[++index] != NULL) && (cmds[index]->CASE == BCKG)){
      execute_command(cmds[index], 1);
    }

    /* exit */
    /*      */
    return 1;
  }

  /* else, we have either a pipe or redirection */
  /*                                            */
  else{

    /* if this is a redirection */
    /*                          */
    if(cmds[0]->CASE != PIPE){
      return redirect(cmds[0]);
    }

    /* this is a pipe */
    /*                */
    return pipe_process(cmds);
  }
}
/*                 */
/* end of function */


/******************************************************************************
#   function: pipe_process
#
#   arguments: command **cmds - pointer to command structs
#
#   returns: int (1 or 0) depending on success/failure
#
#   this function executes one command and stores its output into a pipe
#   for the second command to read in
******************************************************************************/
int pipe_process(command **cmds)
{

  /* file descriptors */
  /*                  */
  int fd[2];
  int s1,s2;

  /* pide the file descriptors */
  /*                           */
  pipe(fd);
  
  /* split up the commands */
  /*                       */
  command *left = cmds[0];

  /* there may be multiple pipes, so get all the commands */
  /*                                                      */
  command **right = (command **) malloc(25*sizeof(command *));

  /* index to grab the commands */
  /*                            */
  int index_r = 0;
  int index_c = 1;

  /* if the right hand command is null */
  /*                                   */
  if(cmds[1] == NULL){
    fprintf(stderr, "%s\n", "shell: must have command at the right");
    return 0;
  }

  /* while we haven't run out of commands */
  /*                                      */
  while(cmds[index_c] != NULL){
    right[index_r++] = cmds[index_c++];
  }

  /* pids for new processes */
  /*                        */
  pid_t p1, p2;
  
  /* if we are only piping two processes */
  /*                                     */
  if(index_r == 1){

    /* create a fork, return if err */
    /*                              */
    if((p1 = fork()) == -1){
      fprintf(stderr, "%s\n", "shell: failed to create fork");
      return 0;
    }

    /* if this is the child process */
    /*                              */
    if(p1 == 0){

      /* send the output to pipe */
      /*                         */
      close(fd[0]);
      dup2(fd[1], STDOUT_FILENO);

      /* execute the command */
      /*                     */
      return execute_command(left, 0);
    }

    /* wait for the child to finish */
    /*                              */
    wait(NULL);
    
    /* if we are in the parent process, create a fork */
    /*                                                */
    if((p2 = fork()) == -1){
      fprintf(stderr, "%s\n", "shell: failed to create fork");
      return 0;
    }

    /* if we are in the child process */
    /*                                */
    if(p2 == 0){

      /* get the input from pipe */
      /*                         */
      close(fd[1]);
      dup2(fd[0], STDIN_FILENO);

      /* execute the command */
      /*                     */
      return execute_command(right[0], 0);
    }

    /* if we are in the parent process */
    /*                                 */
    close(fd[1]);
    close(fd[0]);

    /* wait for all the processes to finish */
    /*                                      */
    waitpid(p1, &s1, WUNTRACED);
    waitpid(p2, &s2, WUNTRACED);
  }

  /* exit gracefully */
  /*                 */
  return 1;
}
/*                 */
/* end of function */


/******************************************************************************
#   function: redirect
#
#   arguments: command *cmd - pointer to single command
#
#   returns: int (1 or 0) depending on success/failure
#
#   this function properly redirects the output of a process
******************************************************************************/
int redirect(command *cmd)
{

  /* file to be opened */
  /*                   */
  int fd;
  int saved_std;
  int std;
  
  /* index of the arguments */
  /*                        */
  int index = -1;
  
  /* get to the end of the args */
  /*                            */
  while(cmd->arg_list[++index] != NULL){
  }

  /* set the file name */
  /*                   */
  char *fname = cmd->arg_list[--index];

  /* set the last arg to NULL */
  /*                          */
  cmd->arg_list[index] = NULL;
  
  /* if this is redirecting in */
  /*                           */
  if(cmd->CASE == STDIN){

    /* save the stdin fd */
    /*                   */
    saved_std = dup(0);
    std = 0;
    
    /* if we failed to open the file to read */
    /*                                       */
    if((fd = open(fname, O_RDONLY, 0755)) < 0){
      fprintf(stderr, "%s\n", "shell: failed to open file");
      return 0;
    }

    /* duplicate the file descriptor */
    /*                               */
    dup2(fd, STDIN_FILENO);
  }

  /* if this is redirecting out */
  /*                            */
  else if(cmd->CASE == STDOUT){

    /* save the stdout fd */
    /*                    */
    saved_std = dup(1);
    std = 1;

    /* if we failed to open the file to write */
    /*                                        */
    if((fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR|S_IWUSR)) < 0){
      fprintf(stderr, "%s\n", "shell: failed to open file");
      return 0;
    }

    /* duplicate the file descriptor */
    /*                               */
    dup2(fd, STDOUT_FILENO);
  }

  /* if this is stdout append mode */
  /*                               */
  else {

    /* save the stdout fd */
    /*                    */
    saved_std = dup(1);
    std = 1;

    /* if we failed to open the file to append */
    /*                                         */
    if((fd = open(fname, O_CREAT | O_RDWR | O_APPEND,
		  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)) < 0){
      fprintf(stderr, "%s\n", "shell: failed to open file");
      return 0;
    }

    /* duplicate the file descriptor */
    /*                               */
    dup2(fd, STDOUT_FILENO);
  }
  
  /* execute the command */
  /*                     */
  execute_command(cmd, 1);

  /* close the file descriptor and restore old fd */
  /*                                              */
  close(fd);
  dup2(saved_std, std);

  /* return sucessfully */
  /*                    */
  return 1;  
}
/*                 */
/* end of function */


/******************************************************************************
#   function: set_redir_case
#
#   arguments: command **cmd
#
#   returns: int (1 or 0) if it successfully set a case
#
#   this function checks to see if the given string is a redirect option
#   and returns a logical status
******************************************************************************/
int set_redir_case(command *cmd, char *option)
{

  /* if this is an empty string */
  /*                            */
  if(strlen(option) < 1){
    return 0;
  }

  /* if there is only one string */
  /*                             */
  if(strlen(option) == 1){

    /* if this is redirecting stdin */
    /*                              */
    if(option[0] == DELIM_REDIR_STDIN){

      /* set the command case to redir stdin */
      /*                                     */
      cmd->CASE = STDIN;
      return 1;
    }

    /* if this is redirecting stdout */
    /*                               */
    else if(option[0] == DELIM_REDIR_STDOUT){

      /* set the command case to redir stdout */
      /*                                      */
      cmd->CASE = STDOUT;
      return 1;
    }

    /* if neither condition is met */
    /*                             */
    else{

      /* return false */
      /*              */
      return 0;
    }
  }

  /* if there are 2 characters */
  /*                           */
  if(strlen(option) == 2){

    /* if this command is >> */
    /*                       */
    if((option[0] == DELIM_REDIR_STDOUT) && (option[1] == DELIM_REDIR_STDOUT)){

      /* set the command case to redir stdout append mode */
      /*                                                  */
      cmd->CASE = STDOUT_APPEND;
      return 1;
    }

    /* if it is not >> */
    /*                 */
    else{

      /* return false */
      /*              */
      return 0;
    }
  }

  /* if there are >2 chars */
  /*                       */
  else{

    /* return false */
    /*              */
    return 0;
  }
}
/*                 */
/* end of function */


/******************************************************************************
#   function: is_option
#
#   arguments: char *command
#
#   returns: int (1 or 0) if it is a command
#
#   this function checks to see if the given string is an option
******************************************************************************/
int is_option(char *command)
{

  /* if there is less than 2 characters */
  /*                                    */
  if(strlen(command) < 2){

    /* this is not an option */
    /*                       */
    return 0;
  }

  /* if this string does not start with "-" */
  /*                                        */
  if(command[0] != DELIM_OPTION){

    /* this is not an option */
    /*                       */
    return 0;
  }

  /* this is an option */
  /*                   */
  return 1;
}
/*                 */
/* end of function */


/******************************************************************************
#   function: parse_cmdline
#
#   arguments: char *commands - pointer to string of commands
#
#   returns: char **parsed_cmds
#
#   this function returns a pointer to strings containing the tokenized
#   commands from the commandline
******************************************************************************/
char **parse_cmdline(char *commands)
{

  /* copy string to an array of chars */
  /*                                  */
  char cmds[100];
  strcpy(cmds, commands);

  /* this will hold all the strings */
  /*                                */
  char **parsed_cmds = malloc(strlen(commands) * sizeof(char *));

  /* space will be used to tokenize string */
  /*                                       */
  const char space[4] = DELIM_WHITESPACE;

  /* begin tokenizing the string by space */
  /*                                      */
  char *token = strtok(cmds, space);

  /* int used to allocate strings to array */
  /*                                       */
  int ind = 0;

  /* continue tokenizing */
  /*                     */
  while(token != NULL){

    /* malloc each pointer */
    /*                     */
    parsed_cmds[ind] = (char *) malloc(strlen(token) * sizeof(char));

    /* set the current index to the current token */
    /*                                            */
    strcpy(parsed_cmds[ind++], token);

    /* continue tokenizing */
    /*                     */
    token = strtok(NULL, space);
  }

  /* if only white space was entered */
  /*                                 */
  if(ind == 0){
    return NULL;
  }

  /* reallocate to prevent memory leak */
  /*                                   */
  parsed_cmds = (char **) realloc(parsed_cmds, (++ind) * sizeof(char *));

  /* set the last pointer to null */
  /*                              */
  parsed_cmds[ind - 1] = NULL;

  /* return the parsed commands */
  /*                            */
  return parsed_cmds;
}
/*                 */
/* end of function */


/******************************************************************************
#   function: arg_parser
#
#   arguments: char **commands - pointer to pointers of strings
#
#   returns: char **parsed_args
#
#   this function returns pointers to structs of the parsed arguments
******************************************************************************/
command **arg_parser(char **commands)
{

  if(commands == NULL){
    return NULL;
  }

  /* allocate an excess number of structs initially */
  /*                                                */
  command **ret;

  /* if we failed to allocate the commands */
  /*                                       */
  if((ret = (command **) malloc(25 * sizeof(command *))) == NULL){

    /* print the error to the user */
    /*                             */
    fprintf(stderr, "%s\n", " FAILED TO ALLOCATE MEMORY FOR COMMANDS");
    exit(0);    
  }

  /* use this int for each word in commands */
  /*                                        */
  int current_arg = -1;
  int current_cmd = -1;
  
  /* boolean to check if this is the command name */
  /*                                              */
  int is_cmd_name = 1;

  /* use these ints to keep track of number of options/args */
  /*                                                        */
  int num_options = -1;
  int num_args = -1;

  /* boolean to check if we need to create a new struct */
  /*                                                    */
  int new_struct = 0;

  /* check to see if we have args after redirection */
  /*                                                */
  int is_arg = 0;
  
  /* allocate the first command */
  /*                            */
  command *current = (command *) malloc(sizeof(command));

  /* initially, the command is in normal mode */
  /*                                          */
  current->CASE = NORMAL;
  
  /* allocate excess number of options and args initially */
  /*                                                      */
  current->options = (char **) malloc(100 * sizeof(char *));
  current->arg_list = (char **) malloc(100 * sizeof(char *));
  
  /* point to the first command */
  /*                            */
  ret[++current_cmd] = current;
  
  /* while we haven't reached the last command */
  /*                                           */
  while(commands[++current_arg] != NULL){

    /* if this is a command name */
    /*                           */
    if(is_cmd_name){

      /* malloc the string */
      /*                   */
      current->name = (char *) malloc(strlen(commands[current_arg]));
      
      /* copy the string to the allocated string */
      /*                                         */
      strcpy(current->name, commands[current_arg]);
      
      /* no longer the command name */
      /*                            */
      is_cmd_name = 0;
    }

    /* if this is a redirection */
    /*                          */
    else if(set_redir_case(current, commands[current_arg])){

      /* next is an argument */
      /*                     */
      is_arg = 1;
      
      /* do nothing */
      /*            */
      continue;
    }

    /* if this a parallel/background command */
    /*                                       */
    else if(commands[current_arg][0] == DELIM_BCKG){

      /* set the case for the current command */
      /*                                      */
      current->CASE = BCKG;

      /* if the next word in the array is NULL */
      /*                                       */
      if(commands[current_arg + 1] == NULL){

	/* there are no more commands */
	/*                            */
	break;
      }

      /* we need to create a new struct */
      /*                                */
      new_struct = 1;      
    }

    /* if this is a pipe instruction */
    /*                               */
    else if(commands[current_arg][0] == DELIM_PIPE){

      /* set the case for the current command */
      /*                                      */
      current->CASE = PIPE;

      /* we need to create a new struct */
      /*                                */
      new_struct = 1;
    }

    /* if none of the conditions are met */
    /*                                   */
    else if(is_arg){

      /* this is an argument */
      /*                     */
      char *arg = (char *) malloc(strlen(commands[current_arg])*sizeof(char));

      /* copy to the arg string */
      /*                        */
      strcpy(arg, commands[current_arg]);

      /* point to the argument */
      /*                       */
      current->arg_list[++num_args] = arg;

      /* set arg to NULL */
      /*                 */
      arg = NULL;
    }

    /* if this is an option */
    /*                      */
    else if(is_option(commands[current_arg]) || !is_arg){

      /* allocate the sting on the heap */
      /*                                */
      char *option = (char *) malloc(strlen(commands[current_arg]));

      /* copy the command to the string */
      /*                                */
      strcpy(option, commands[current_arg]);

      /* point to the string */
      /*                     */
      current->options[++num_options] = option;

      /* set option to null */
      /*                    */
      option = NULL;      
    }

    /* if we need to make a new struct */
    /*                                 */
    if(new_struct){

      /* reset boolean value */
      /*                     */
      is_arg = 0;

      /* if there were no options, set to null, else realloc */
      /*                                                     */
      if(num_options == -1){
	current->options[0] = NULL;
      }else{
	num_options += 2;
	current->options = realloc(current->options,sizeof(char *)*num_options);
	current->options[num_options - 1] = NULL;
      }

      /* if there were no arguments, set to null, else realloc */
      /*                                                       */
      if(num_args == -1){
	current->arg_list[0] = NULL;
      }else{
	num_args += 2;
	current->arg_list = realloc(current->arg_list,sizeof(char *)*num_args);
	current->arg_list[num_args - 1] = NULL;
      }
      
      /* reset to -1 */
      /*             */
      num_options = -1;
      num_args = -1;

      /* set the current pointer to the new command */
      /*                                            */
      current = (command *) malloc(sizeof(command));

      /* initially, the command is in normal mode */
      /*                                          */
      current->CASE = NORMAL;

      /* allocate excess for options and arguments */
      /*                                           */
      current->options = (char **) malloc(100 * sizeof(char *));
      current->arg_list = (char **) malloc(100 * sizeof(char *));

      /* point to the new command */
      /*                          */
      ret[++current_cmd] = current;

      /* the next word will be a command */
      /*                                 */
      is_cmd_name = 1;
      
      /* we no longer need to create a new struct */
      /*                                          */
      new_struct = 0;
    }
  }

  /* adjust number of cmds, options, args */
  /*                                      */
  current_cmd += 2;
  
  /* if there were no options, set to null, else realloc */
  /*                                                     */
  if(num_options == -1){
    current->options[0] = NULL;    
  }else{
    num_options += 2;
    current->options = realloc(current->options,sizeof(char *)*num_options);
    current->options[num_options - 1] = NULL;
  }

  /* if there were no arguments, set to null, else realloc */
  /*                                                       */
  if(num_args == -1){
    current->arg_list[0] = NULL;
  }else{
    num_args += 2;
    current->arg_list = realloc(current->arg_list,sizeof(char *)*num_args);
    current->arg_list[num_args - 1] = NULL;
  }

  /* reallocate the return struct to prevent mem leak */
  /*                                                  */
  ret = realloc(ret, sizeof(command *) * current_cmd);

  /* set the last element of the array to NULL */
  /*                                           */
  ret[current_cmd - 1] = NULL;
  
  /* return the array of commands */
  /*                              */
  return ret;
}
/*                 */
/* end of function */


/******************************************************************************
#   function: exec_batch
#
#   arguments: char *file - file name 
#
#   returns: none
#
#   this function executes the commands in a file
******************************************************************************/
void exec_batch(char *file)
{

  /* try to open the file to read */
  /*                              */
  FILE *batch;
  if((batch = fopen(file, "r")) == NULL){
    fprintf(stderr, "%s\n", "shell: failed to open file in batch mode");
    return;
  }
  
  /* vars to be used to read in commands */
  /*                                     */
  size_t max_buff_size = 0;
  char *current_command = NULL;
  char **parsed_cmds;
  command **parsed_args;

  /* while we the line is not null */
  /*                               */
  while((getline(&current_command, &max_buff_size, batch)) != -1){

    /* pointer to each command given */
    /*                               */
    parsed_cmds = parse_cmdline(current_command);

    /* pointer to struct of every commnd */
    /*                                   */
    parsed_args = arg_parser(parsed_cmds);

    /* process and execute the commands */
    /*                                  */
    process_args(parsed_args);

    /* clear the current command to be used again */
    /*                                            */
    free(current_command);
    free(parsed_args);
    current_command = NULL;
    parsed_args = NULL;
  }
}

/*****************************************************************************/
/*---------------------------------end of file-------------------------------*/

