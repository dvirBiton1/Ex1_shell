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
#include <cstdlib>
//#include <ncurses.h>

#define KEY_UP "\x1b[A"
#define KEY_DOWN "\x1b[B"

using namespace std;
char PROMPT_BUFFER[30];

void sigint_handler(int signum) {
    std::cout << "You typed Control-C!" << std::endl << std::flush;
    printf("%s", PROMPT_BUFFER);
}

int main() {
    signal(SIGINT, sigint_handler);
    char command[1024];
    char *token;
    char *outfile;
    int i, fd, amper, redirect, retid, status;
    char *argv[10];
    strcpy(PROMPT_BUFFER, "hello: ");
    map<int, string> last_commands;
    map <string, string> vars;
    int last_command_key = -1;


    while (1) {
        printf("%s", PROMPT_BUFFER);
        fgets(command, 1024, stdin);
        command[strlen(command) - 1] = '\0';
        last_command_key += 1;
        last_commands.insert({last_command_key, command});
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

        if (!strcmp(argv[0], "quit")) {
            exit(0);
        }
        if (!strcmp(command, KEY_UP)) {
            cout << "\r" << flush;
            char *com = const_cast<char *>(last_commands[last_command_key - 1].c_str());
            std::cout << com << std::endl;
        }
        if (!strcmp(command, KEY_DOWN)) {
            cout << "ciiii" << endl;
        }

        if (!strcmp(argv[0], "!!")) {
            char *com = const_cast<char *>(last_commands[last_command_key - 1].c_str());
            std::cout << com << std::endl;
            std::system(com);
        }

//        if (! strcmp(argv[0], "cd")){
//            std::string str_command(command);
//            string sliced_cd = str_command.substr(2);
//            char * s_cd = const_cast<char*>(sliced_cd.c_str());
//            chdir(s_cd);
//        }

        /* Does command line start with echo */
        if (!strcmp(argv[0], "echo")) {
            int argv_i = 1;
            while (argv[argv_i] != NULL) {
                if (argv[argv_i][0] == '$') {
                    std::string str_var(argv[argv_i]);
                    string sliced_var = str_var.substr(1);
                    cout << vars.at(sliced_var) << " ";
                } else {
                    printf("%s", argv[argv_i]);
                }
                argv_i++;
            }
        }

        if (argv[0][0] == '$') {
            cout << "ciii" << endl;
            if (!strcmp(argv[1], "=")) {
                cout << "cii222i" << endl;
                std::string str_var(argv[0]);
                string sliced_var = str_var.substr(1);
                vars.insert({sliced_var, argv[2]});
            }
        }

        /* Does command line end with & */
        if (!strcmp(argv[i - 1], "&")) {
            amper = 1;
            argv[i - 1] = NULL;
        } else
            amper = 0;

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

        if (fork() == 0) {
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
        if (amper == 0)
            retid = wait(&status);
    }
}

