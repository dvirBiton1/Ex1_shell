#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
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
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <sstream>

using namespace std;

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


int if_else(const char *command) {
    pid_t pid;
    int status;

    if (command == NULL) {
        return 1;
    }

    pid = fork();
    if (pid == -1) {
        return -1;
    } else if (pid == 0) {
        // Child process
        const char *argv[] = {"sh", "-c", command, NULL};
        execvp("/bin/sh", (char **)argv);
        _exit(EXIT_FAILURE);
    } else {
        // Parent process
        do {
            if (waitpid(pid, &status, 0) == -1) {
                if (errno != EINTR) {
                    return -1;
                }
            } else {
                return status;
            }
        } while (1);
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
    const int MAX_COMMANDS = 30;  // Maximum number of commands to store in memory
    int i, fd, amper, redirect, retid, status;
    char *argv[10];
    char PROMPT_BUFFER[30];
    std::vector <string> commandHistory; // Vector to store command history
    int currentCommandIndex = -1;
    strcpy(PROMPT_BUFFER, "hello: ");
    signal(SIGINT, sigint_handler);
    while (1) {
        printf("%s", PROMPT_BUFFER);
        strcpy(last_command, temp_command);
        fgets(command, 1024, stdin);
        command[strlen(command) - 1] = '\0';
        strcpy(temp_command, command);
        string command_s = command;

        if ((strstr(command, " | ") != NULL) && (strstr(command, "if") == NULL)) {

            vector <string> commands;

            char *token = strtok(command, "|");
            while (token != NULL) {
                string copy_command = "" + string(token);
                commands.push_back(copy_command);
                token = strtok(NULL, "|");
            }

            // Execute all commands
            int fd[2];
            int last_in = STDIN_FILENO;
            for (int i = 0; i < commands.size(); i++) {
                // Split command string into array of arguments
                char *com = const_cast<char *>(commands[i].data());
                char *arg = strtok(com, " ");
                vector <string> args;
                while (arg != NULL) {
                    args.push_back(arg);
                    arg = strtok(NULL, " ");
                }

                // Create pipe
                if (i != commands.size() - 1) {
                    if (pipe(fd) == -1) {
                        perror("pipe failed");
                        exit(EXIT_FAILURE);
                    }
                }
                // Execute command
                pid_t pid = fork();
                if (pid == 0) {
                    // Redirect input from previous command (if not first command)
                    if (last_in != STDIN_FILENO) {
                        if (dup2(last_in, STDIN_FILENO) == -1) {
                            perror("dup2 failed");
                            exit(EXIT_FAILURE);
                        }
                        close(last_in);
                    }

                    // Redirect output to next command (if not last command)
                    if (i != commands.size() - 1) {
                        if (dup2(fd[1], STDOUT_FILENO) == -1) {
                            perror("dup2 failed");
                            exit(EXIT_FAILURE);
                        }
                        close(fd[1]);
                    }

                    // Execute command

                    std::vector<const char *> cargs;
                    cargs.reserve(args.size() + 1);
                    for (const auto &arg: args) {
                        cargs.push_back(arg.c_str());
                    }
                    cargs.push_back(nullptr);
                    execvp(cargs[0], const_cast<char **>(cargs.data()));
                    perror("execvp failed");
                    exit(EXIT_FAILURE);
                } else {
                    // Close input of previous command (if not first command)
                    if (last_in != STDIN_FILENO) {
                        close(last_in);
                    }

                    // Close output of this command (if not last command)
                    if (i != commands.size() - 1) {
                        close(fd[1]);
                    }

                    // Save input for next command (if not last command)
                    if (i != commands.size() - 1) {
                        last_in = fd[0];
                    }

                    // Wait for command to finish
                    wait(NULL);
                }
            }
            continue;
        }


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
        bool flag_while = false;
        while (argv[0][0] == 27 && argv[0][1] == 91) { // if the first value is esc and the next two are '['
            flag_while = true;
            string inputString;
            printf("\033[1A"); // line up
            printf("\x1b[2K"); // delete line
            char arrowKey = argv[0][2];
            if (arrowKey == 65) { // up arrow
                if (commandHistory.size() == 0) {
                    continue;
                }
                if (currentCommandIndex == MAX_COMMANDS) {
                    currentCommandIndex = 0;
                } else {
                    currentCommandIndex++;
                }
            } else if (arrowKey == 66) { // down arrow
                if (commandHistory.size() == 0) {
                    continue;
                }

                if (currentCommandIndex == 0) {
                    currentCommandIndex = commandHistory.size() - 1;
                } else {
                    currentCommandIndex--;
                }
            }
            if (currentCommandIndex >= 0 && currentCommandIndex < commandHistory.size()) {
                inputString = commandHistory[currentCommandIndex];
                const char *inputChars = inputString.c_str();

                int i = 0;
                char *tok = strtok((char *) inputChars, " ");
                while (tok != NULL && i < 9) {
                    argv[i] = tok;
                    tok = strtok(NULL, " ");
                    i++;
                }
                argv[i] = NULL;
            } else {
                continue;
            }

            i = 0;
            while (argv[i] != NULL) {
                cout << argv[i] << " ";
                i++;
            }
            cout << endl;
            fgets(command, 1024, stdin);
            if (strlen(command) >= 3 && command[0] == '\x1b' && command[1] == '[') {
                int i = 0;
                char *tok = strtok(command, " ");
                while (tok != NULL && i < 9) {
                    argv[i] = tok;
                    tok = strtok(NULL, " ");
                    i++;
                }
                argv[i] = NULL;
                printf("\033[1A"); // line up
                printf("\x1b[2K"); // delete line
            } else if (command[0] == '\n') {
                break;
            }
        }
        if (!flag_while) {
            commandHistory.push_back(command_s);
            if (commandHistory.size() > MAX_COMMANDS) {
                commandHistory.erase(commandHistory.begin());
            }
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

        if (strcmp(argv[0], "cd") == 0) {
            chdir(argv[1]);
            continue;
        }
        if (std::strcmp(argv[0], "!!") == 0) {
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

        if (!strcmp(argv[0], "read")) {
            char buffer[100];
            char buffer2[100];
            strcpy(buffer, argv[1]);
            scanf("%s", buffer2);
            setenv(buffer, buffer2, 1);
            continue;
        }

        if (!strcmp(argv[0], "echo")) {
            if (strcmp(argv[1], "$?") == 0) {
                printf("Last command exited with status %d\n", status);
                continue;
            }
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
        if ((i >= 2) && (!strcmp(argv[1], "=")) && (!strcmp(argv[0], "prompt"))) {
            strcpy(PROMPT_BUFFER, argv[2]);
            continue;
        }

        if (!strcmp(argv[0], "if")) {
            char externalCommand[1024] = "";
            int j = 0;
            while (argv[j] != NULL) {
                strcat(externalCommand, argv[j]);
                strcat(externalCommand, " ");
                j++;
            }
            strcat(externalCommand, "\n");
            char buf[1024] = "";
            fgets(buf, 1024, stdin);
            while (strcmp(buf, "fi\n")) {
                strcat(externalCommand, buf);
                strcat(externalCommand, " ");
                memset(buf, 0, sizeof(buf));
                fgets(buf, 1024, stdin);
            }
            strcat(externalCommand, "fi\n");
            if_else(externalCommand);
            continue;
        }

        /* Does command line end with & */
        if ((i >= 1) && (!strcmp(argv[i - 1], "&"))) {
            amper = 1;
            argv[i - 1] = NULL;
        } else
            amper = 0;
        if (i >= 2) {
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
        }

        /* for commands not part of the shell command language */
        if (fork() == 0) {
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
        }
    }
}
