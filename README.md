# Shell Program
This program creates a shell in C. There are a few built in commands:<br/>
<br />
cd: changes the current directory<br />
dir: lists everything in the current directory<br />
environ: prints all the env variables<br />
echo: prints out the user's input<br />
help: prints out a help file<br />
pause: suspends shell execution until newline entered<br />
exit: exits the shell<br />
path: adds PATH value to search for executables <br />
<br />

For the remaining commands, these are executed by forking out the process. This shell supports piping the output of one 
command to be used as the input for the next command (i.e: cat foo.txt | wc -l). This program also supports redirecting 
output to a file or redirecting input from a file.
