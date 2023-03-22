#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGS 10
#define MAX_HISTORY 100

char prompt[10] = "input";
char *history[MAX_HISTORY];
int history_index = 0;

void handle_signal(int signal) {
    printf("\nhello %s> ", prompt);
}

void execute_command(char **argv, int background, int redirect_type, char *outfile) {
    pid_t pid;
    int status;

    pid = fork();

    if (pid == 0) {
        // child process
        if (redirect_type == 1) {
            // redirect stdout to a file
            int fd = open(outfile, O_CREAT | O_WRONLY | O_APPEND, 0644);
            dup2(fd, STDOUT_FILENO);
            close(fd);
        } else if (redirect_type == 2) {
            // redirect stderr to a file
            int fd = open(outfile, O_CREAT | O_WRONLY | O_APPEND, 0644);
            dup2(fd, STDERR_FILENO);
            close(fd);
        }
        execvp(argv[0], argv);
        // if execvp returns, there was an error
        printf("%s: command not found\n", argv[0]);
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // error forking
        perror("fork");
    } else {
        // parent process
        if (background == 0) {
            // wait for child to finish
            waitpid(pid, &status, 0);
        }
    }
}

char **parse_command(char *command, int *background, int *redirect_type, char **outfile) {
    char **argv = (char **) malloc(MAX_ARGS * sizeof(char *));
    int argc = 0;

    char *token = strtok(command, " ");

    while (token != NULL) {
        if (*token == '&') {
            *background = 1;
        } else if (*token == '>' && *(token + 1) == '>') {
            *redirect_type = 1;
            *outfile = strtok(NULL, " ");
        } else if (*token == '2' && *(token + 1) == '>') {
            *redirect_type = 2;
            *outfile = strtok(NULL, " ");
        } else {
            argv[argc++] = token;
        }

        token = strtok(NULL, " ");
    }

    argv[argc] = NULL;
    return argv;
}

void print_history() {
    for (int i = 0; i < history_index; i++) {
        printf("%d: %s\n", i, history[i]);
    }
}

char *get_history(int index) {
    if (index >= 0 && index < history_index) {
        return history[index];
    } else {
        return NULL;
    }
}

int main() {
    char command[MAX_COMMAND_LENGTH];
    int background, redirect_type;
    char *outfile;
    char **argv;

    signal(SIGINT, handle_signal);

    while (1) {
        printf("hello %s> ", prompt);
        fflush(stdout);

        if (fgets(command, MAX_COMMAND_LENGTH, stdin) == NULL) {
            // end of file
            printf("\n");
            break;
        }

        // remove newline character from command
