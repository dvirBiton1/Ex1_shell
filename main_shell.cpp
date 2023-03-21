#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>

int main() {
    char command[1024];
    char *token;
    char *outfile;
    int i, fd, amper, redirect, retid, status;
    char *argv[10];
    char PROMPT_BUFFER[30];
    strcpy(PROMPT_BUFFER, "hello: ");

    while (1)
    {
        printf("%s", PROMPT_BUFFER);
        fgets(command, 1024, stdin);
        command[strlen(command) - 1] = '\0';

        /* parse command line */
        i = 0;
        token = strtok (command," ");
        while (token != NULL)
        {
            argv[i] = token;
            token = strtok (NULL, " ");
            i++;
        }
        argv[i] = NULL;

        /* Is command empty */
        if (argv[0] == NULL)
            continue;

        /* Does command line start with echo */
        if (! strcmp(argv[0], "echo")){
            for (int index_command=5; index_command<strlen(command); index_command++){
                printf("%d", command[i]);
            }
        }

        /* Does command line end with & */
        if (! strcmp(argv[i - 1], "&")) {
            amper = 1;
            argv[i - 1] = NULL;
        }
        else
            amper = 0;

        /* Does command line have prompt = */
        if (! strcmp(argv[i - 2], "=")) {
            if (! strcmp(argv[i - 3], "prompt")) {
                strcpy(PROMPT_BUFFER, argv[i-1]);
            }
        }

        if (! strcmp(argv[i - 2], ">")) {
            redirect = 1;
            argv[i - 2] = NULL;
            outfile = argv[i - 1];
        }
        else if (! strcmp(argv[i - 2], "2>")) {
            redirect = 2;
            argv[i - 2] = NULL;
            outfile = argv[i - 1];
        }
        else if (! strcmp(argv[i - 2], ">>")) {
            redirect = 3;
            argv[i - 2] = NULL;
            outfile = argv[i - 1];
        }
        else
            redirect = 0;

        /* for commands not part of the shell command language */

        if (fork() == 0) {
            /* redirection of IO ? */
            if (redirect) {

                switch (redirect) {
                    case 1:
                        printf("1\n");
                        fd = creat(outfile, 0660);
                        close (STDOUT_FILENO) ;
                        dup(fd);
                        close(fd);
                        /* stdout is now redirected */
                        break;
                    case 2:
                        printf("2\n");
                        fd = creat(outfile, 0660);
                        close (STDERR_FILENO) ;
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
