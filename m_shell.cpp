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


using namespace std;
char PROMPT_BUFFER[30];
int control_c = 0;

void sigint_handler(int signum) {
    std::cout << "You typed Control-C!" << std::endl << std::flush;
}

int main() {
    signal(SIGINT, sigint_handler);
    char command[1024];
    char *token;
    char *outfile;
    int i, fd, amper, redirect, retid, status;
    char *argv[10];
    strcpy(PROMPT_BUFFER, "hello: ");

    while (1) {
        printf("%s ", PROMPT_BUFFER);
        fgets(command, 1024, stdin);
        command[strlen(command) - 1] = '\0';

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

        /* Does command line end with & */
        if (!strcmp(argv[i - 1], "&")) {
            amper = 1;
            argv[i - 1] = NULL;
        } else
            amper = 0;

        if (!strcmp(argv[0], "quit")) {
            exit(0);
        }

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

        if (!strcmp(argv[0], "read")) {
            char buffer[100];
            char buffer2[100];
            strcpy(buffer, argv[1]);
            scanf("%s", buffer2);
            setenv(buffer, buffer2, 1);
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





        /* Does command line have prompt = */
        if (!strcmp(argv[i - 2], "=")) {
            if (!strcmp(argv[i - 3], "prompt")) {
                strcpy(PROMPT_BUFFER, argv[i - 1]);
            }
        }

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




        /* for commands not part of the shell command language */
//-------------------------------------------------------------------------------------------------------------------------------
        if (fork() == 0) {
            if (control_c) {
                exit(0);
            }
            /* redirection of IO ? */
            if (redirect) {

                switch (redirect) {
                    case 1:
                        printf("1\n");
                        fd = creat(outfile, 0660);
                        close(STDOUT_FILENO);
                        dup(fd);
                        close(fd);
                        /* stdout is now redirected */
                        break;
                    case 2:
                        printf("2\n");
                        fd = creat(outfile, 0660);
                        close(STDERR_FILENO);
                        dup(fd);
                        close(fd);
                        /* stderr is now redirected */
                        break;
                    case 3:
                        printf("3\n");
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
            execvp(argv[0], argv);
        }
        /* parent continues here */
        if (control_c) {
            control_c = 0;
        }

        if (amper == 0)
            retid = wait(&status);
    }
}

