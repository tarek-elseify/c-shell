MYSHELL(1)                                                           General Commands Manual                                                           MYSHELL(1)

NAME
       myshell - C shell to perform basic commands and executions

SYNOPSIS
       myshell
       myshell [file]

DESCRIPTION
       This shell program uses multi-processing to run other executable files. It includes nine internal functions that exist within the shell program, and is
       able to execute files in the PATH. There are two modes that this shell can be executed in. If no arguments are given, the shell is run in interactive
       mode where a prompt is displayed and a user is able to enter their commands. If an argument is given, the shell runs in batch mode and executes all the
       commands in the given file. This shell supports I/O redirection, piping, and background execution. The commands are tokenized by whitespace, so any extra
       spaces or tabs will not affect the processing of the arguments. 

Startup and Shutdown
       In interactive mode, the shell begins reading commands from the terminal, prompting with `myshell: '. The shell repeatedly gets the line entered by the
       user, and tokenizes the words by whitespace. An algorithm then determines the name of the program, the options, arguments, and any special cases and
       stores them in a struct. The special cases dictate the execution of the commands (which will be explained later on). To exit the program, the user can
       enter 'exit'. This will fail if this command is provided with any options or arguments. Alternatively, the ^C character will also terminate the program.


       In batch more, the shell does not display any prompt. It parses through all the lines of the file, and executes each command one by one. These commands
       must be seperated by newline (each line is still tokenized by whitespace, so spaces and tabs do not effect the program). The program will continue to
       execute the commands until it has reached the EOF. On exiting, the program simply terminates without any printed output.

Built-In Commands
       There are a total of nine built in commands. These commands are internal to the shell, and do not have their own executable files. The main advatanges
       of these commands is the lack of overhead of copying and creating a new process. Also, changing the directory is specific to the process executing the
       command, so the cd command must be internal. Below is a list of these commands, and their usage.

       	    cd [path]
	       Changes directory of the current process to the given directory. If no directory is provided, then this will default to the home directory
	       If an invalid directory is provided, the program will error and stay at the current working directory. After the program has changed into
	       the designated directory, it will print the directory out to the user.

            clr
	       Clears the screen of all characters. If an option is provided, the program will error.

	    dir [path]
	       Prints out the contents of the given directory. If no directory is provided, it will output the content of the current working directory.
	       If an invalid directory is provided, then the program will error.

	    echo [comment]
	       Displays the given comment on the display verbatim and follows it by a newline.

            environ
	       Prints out all the environment strings. If any options/arguments are provided, then the program will error.

            help
	       Prints this menu using the more filter. This command is executed by forking the more command and passing this file as the argument.

	    pause
	       Pauses the operation of the shell program until a newline has been entered. If any options/arguments are provided, the program will error.

	    quit
	       Terminates the execution of the shell program. If any options/arguments are provided, then the program will error.
	       
       The output of any of these internal commands can be redirected both in and out. More on this in the next section.

I/O Redirection
       Any command input to the shell can have its output or input redirected into or from a file. This is especially useful if the data you need comes from or
       needs to be put into a file. There are three methods of I/O Redirection and they are signified by these symbols '> >> <'. To write the output of a
       program into a file, the user has an option of using the '>' or '>>' characters followed by the file name to be written. For example, the command
       ls -l > out.txt will write the output of 'ls -l' into the file 'out.txt'. If the file does not exist, it will be created (given that the directory
       provided exists). If there is any content inside the file, it will become overwritten. If the user does not wish to overwrite the content of the file, and
       instead append to it, then the characters '>>' should be used. This will ensure that the output goes at the end of the file if there is content inside the
       file. If the file does not exist, it will create the file for the user. To redirect the contents of a file to the input of a program, the '<' character
       must be used. This will redirect the standard input for a command. If the file does not exist, the program will error.

Process Piping
       This shell supports piping two processes. When two processes are piped, the output of one program becomes the input of another. This is done by holding
       the output of one file into a pipe, and having the second process read from it. To pipe two processes together, simply seperate the two commands by the
       '|' character. For example, 'ls -l | wc -l' would count the number of items in your current working directory.

Background Execution
       Multiple processes can be executed simultaneously in the background. To run a command in the background, the user must enter the command followed by the
       '&' symbol. This can be done with multiple commands. For example, to run three commands in the background, the user would enter 'cmd1 & cmd2 & cmd3 &'.
       When these commands are run in the background, the order of their execution is not guaranteed. Any one process, including the shell, may finish first.
       Users can continue to execute command once the prompt shows up again.

Tarek Elseify                                                           11 March 2019                                                                  MYSHELL(1)
