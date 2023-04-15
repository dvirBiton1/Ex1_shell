# Shell Program
This is a shell program that can execute various commands and perform additional features.
****************************************
Running the Program:
To run the program, navigate to the directory where the program is saved and execute the following command in the terminal:
make && ./myshell
*** Run "make clean" to delete the myshell executable file ***

Features
The program can perform the following features:  

Execute commands entered by the user, such as ls -l.  

Execute commands in the background using &, such as ls -l &.  

Redirect output to a file using >, such as ls -l > file.  

Redirect errors to stderr using 2>, such as ls -l nofile 2> mylog.  

Append output to an existing file using >>, such as ls -l >> mylog.  

Built-in commands such as continue, which resumes the program after a pause.  

Change the prompt using the prompt command followed by a space and the desired prompt, such as prompt = myprompt.
Use the echo command to print arguments, such as echo abc xyz.
Use the echo $? command to print the status of the last executed command.
Change the current working directory using the cd command followed by a space and the desired directory, such as cd mydir.
Use !! to repeat the last command.
Use quit to exit the program.
Handle Control-C by printing the message "You typed Control-C!" and not exiting if the program is running another process.
Chain multiple commands together using |, with dynamic allocation of argv for each command.
Create and use variables using the format $variablename = value, such as $person = David. Variables can be echoed using $ followed by the variable name, such as echo $person.
Use read to prompt the user to enter a string and store the input in a variable.
View command history using the up and down arrow keys.
Use flow control statements such as if and else to execute commands conditionally.

Authors
This program was created by Ohad Shirazi and Dvir Biton.
