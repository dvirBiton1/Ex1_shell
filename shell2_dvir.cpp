#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>
#include <iostream>
#include <map>

using namespace
std;

void redirect_handler(int redirect, int fd, char *outfile) {
    switch (redirect) {
        case 1:
            // printf("1\n");
            fd = creat(outfile, 0660);
            close(STDOUT_FILENO);
            dup(fd);
            close(fd);
            /* stdout is now redirected */
            break;
        case 2:
            // printf("2\n");
            fd = creat(outfile, 0660);
            close(STDERR_FILENO);
            dup(fd);
            close(fd);
            /* stderr is now redirected */
            break;
        case 3:
            // printf("3\n");
            fd = open(outfile, O_WRONLY | O_CREAT | O_APPEND, 0660);
            if (fd < 0) {
                perror("open");
                exit(EXIT_FAILURE);
            }
            close(STDOUT_FILENO);
            dup(fd);
            close(fd);
            break;
    }
}

void sigint_handler(int sig) {
    printf("You typed Control-C!\n");
}

int main() {
    char command[1024];
    char *token;
    char *outfile;
    char last_command[1024] = ""; // Initialize to empty string for last_command
    char temp_command[1024] = ""; // Initialize to empty string for last_command
    int i, fd, amper, redirect, retid, status;
    char *argv[10];
    char PROMPT_BUFFER[30];
    strcpy(PROMPT_BUFFER, "hello: ");
    signal(SIGINT, sigint_handler);
    while (1) {
        printf("%s", PROMPT_BUFFER);
        strcpy(last_command, temp_command);
        fgets(command, 1024, stdin);
        command[strlen(command) - 1] = '\0';
        strcpy(temp_command, command);
        // printf("command: %s\n", command);
        // printf("temp_command: %s\n", temp_command);
        // printf("last_command: %s\n", last_command);
        /* parse command line */
        i = 0;
        token = strtok(command, " ");
        while (token != NULL) {
            argv[i] = token;
            token = strtok(NULL, " ");
            i++;
        }
        argv[i] = NULL;

        /* Is command empty */
        if (argv[0] == NULL)
            continue;


        if (argv[0][0] == '$') {
            if ((!strcmp(argv[1], "=")) && (argv[2] != NULL)) {
                std::string str_var(argv[0]);
                string sliced_var = str_var.substr(1);
                if (setenv(sliced_var.c_str(), argv[2], 1) != 0) {
                    cout << "failed!" << endl;
                } else {
                    cout << endl;
                }
            } else {
                cout << "wrong command!" << endl;
            }
            continue;
        }


        /* Does command line have prompt = */
        if ((!strcmp(argv[i - 2], "=")) && (!strcmp(argv[i - 3], "prompt"))) {
            strcpy(PROMPT_BUFFER, argv[i - 1]);
            continue;
        }
        /* Does command line end with & */
        if (!strcmp(argv[i - 1], "&")) {
            amper = 1;
            argv[i - 1] = NULL;
        } else
            amper = 0;

        if (!strcmp(argv[i - 2], ">")) {
            redirect = 1;
            argv[i - 2] = NULL;
            outfile = argv[i - 1];
        } else if (!strcmp(argv[i - 2], "2>")) {
            redirect = 2;
            argv[i - 2] = NULL;
            outfile = argv[i - 1];
        } else if (!strcmp(argv[i - 2], ">>")) {
            redirect = 3;
            argv[i - 2] = NULL;
            outfile = argv[i - 1];
        } else
            redirect = 0;


        if (!strcmp(argv[0], "read")) {
            char buffer[100];
            char buffer2[100];
            strcpy(buffer, argv[1]);
            scanf("%s", buffer2);
            setenv(buffer, buffer2, 1);
            continue;
        }

        if (strcmp(argv[0], "echo") == 0 && strcmp(argv[1], "$?") == 0) {
            printf("Last command exited with status %d\n", status);
            continue;
        }

        if (!strcmp(argv[0], "echo")) {
            int i = 1; // start from the second argument
            while (argv[i] != NULL) {
                if (argv[i][0] == '$') {
                    char *var_value = getenv(argv[i] + 1);
                    if (var_value != NULL) {
                        cout << var_value << " ";
                    } else {
                        cout << argv[i] << " ";
                    }
                } else {
                    cout << argv[i] << " ";
                }
                i++; // move to the next argument
            }
            cout << endl;
            continue;
        }

        if (strcmp(argv[0], "cd") == 0) {
            chdir(argv[1]);
            continue;
        }
        if (strcmp(argv[0], "!!") == 0) {
            /* If the user enters "!!", execute the last command */
            if (strlen(last_command) == 0) {
                printf("No previous command.\n");
            } else {
                printf("Executing last command: %s\n", last_command);
                system(last_command);
            }
            continue;
        }
        if (!strcmp(argv[0], "quit")) {
            exit(0);
        }
        /* for commands not part of the shell command language */
        if (fork() == 0) {
            printf("Child process\n");
            signal(SIGINT, SIG_DFL);
            /* redirection of IO ? */
            if (redirect) {
                redirect_handler(redirect, fd, outfile);
            }
            execvp(argv[0], argv);
        } else {
            /* parent continues here */
            if (amper == 0)
                retid = wait(&status);
            // printf("Father proccess\n");
        }
    }
}
