#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <limits.h>
#include "fsh.h"
#include "parser.h"


#define AC_MAGENTA "\x1b[35m"
#define BOLD "\x1B[1m"
#define END_BOLD "\x1B[0m"


void fsh_exit(void) {
    exit(EXIT_SUCCESS);
}

void fsh_help(void) {
    printf("This is FAKNG Shell\n\n");
    printf("Here some of built-in functions:\n\n");
    printf("help: displays this menu\n\n");
    printf("cd: built-in change dir function\n\n");
    printf("exit: exit shell\n\n");
    printf("setenv VAR_NAME /dir/path/: add new env to PATH\n\n");
}

void fsh_echo(char **cmd) {
    if (cmd[1][0] == '"') {
        for (int i = 1; i < strlen(cmd[1]) - 1; i++) {
            printf("%c", cmd[1][i]);
        }
        puts("");
        return;
    }
    if (cmd[1][0] == '$') {
        char *env_var[] = {"PATH", "USER", "SHELL", "HOME", "PWD", "_", "LANG", "TERM"};
        for (int i = 0; i < strlen(cmd[1]); i++) {
            cmd[1][i] = cmd[1][i + 1];
        }
        for (int i = 0; i < 8; i++) {
            if (strcmp(cmd[1], env_var[i]) == 0) {
                printf("%s\n", getenv(env_var[i]));
                return;
            }
        }
    }
    exit(EXIT_FAILURE);
}

void fsh_setenv(char **cmd) {
    if (cmd[1] == NULL || cmd[2] == NULL) {
        return;
    }
    setenv(cmd[1], cmd[2], 0);
    return;
}

void fsh_cd(char **dir) {
    char *home = getenv("HOME");   
    if(dir[1] == NULL) {
        if (chdir(home) != 0)
            perror("cd");
    }
    else if ((strcmp(dir[1], "~") == 0) || (strcmp(dir[1], "~/") == 0)) {
        if (chdir(home) != 0)
            perror("cd");
    }
    else if(chdir(dir[1]) != 0)
        perror("cd");
}

void fsh_execute(char **cmd) {
    if (strcmp(cmd[0], "exit") == 0) {
        fsh_exit();
        return;
    }
    if (strcmp(cmd[0], "help") == 0) {
        fsh_help();
        return;
    }
    if (strcmp(cmd[0], "cd") == 0) {
        fsh_cd(cmd);
        return;
    }
    if ((strcmp(cmd[0], "echo") == 0 && cmd[1][0] == '"') ||
        (strcmp(cmd[0], "echo") == 0 && cmd[1][0] == '$')) {
        fsh_echo(cmd);
        return;
    }
    if (strcmp (cmd[0], "setenv") == 0) {
        fsh_setenv(cmd);
        return;
    }
    pid_t pid = fork(); 
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        if (execvp(cmd[0], cmd) < 0) {
            printf("fsh: Command not found: %s\n", cmd[0]);
        }
        exit(EXIT_FAILURE);
    } else {
        wait(NULL);
    }
}

void fsh_exec1(char ***cmd) { // executes "&&" bundle
    int status = 0;
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        _exit(1);
    }
    if (pid == 0) {
        execvp(cmd[0][0], cmd[0]);
        perror("fsh");
        _exit(1);
    }
    wait(&status);
    if (WIFEXITED(status)) {
        if (!WEXITSTATUS(status)) {
            pid = fork();
            if (pid == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            }
            if (pid == 0) {
                execvp(cmd[1][0], cmd[1]);
                perror("fsh");
                exit(EXIT_FAILURE);
            }
            wait(NULL);
        }
    }
}

void fsh_exec2(char ***cmd) { // executes "||" bundle
    int status = 0;
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        execvp(cmd[0][0], cmd[0]);
        perror("fsh");
        exit(EXIT_FAILURE);
    }
    wait(&status);
    if (WIFEXITED(status) || WIFSIGNALED(status)) {
        if (WEXITSTATUS(status) || WIFSIGNALED(status)) {
            pid = fork();
            if (pid == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            }
            if (pid == 0) {
                execvp(cmd[0][0], cmd[0]);
                perror("fsh");
                exit(EXIT_FAILURE);
            }
            wait(NULL);
        }
    }
}

void fsh_exec3(char ***cmd) { // executes "<" redirection
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        int fd = open(cmd[1][0], O_EXCL | O_RDONLY);
        if (fd == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }
        if (dup2(fd, STDIN_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        close(fd);
        execvp(cmd[0][0], cmd[0]);
        perror("fsh");
        exit(EXIT_FAILURE);
    }
    wait(NULL);
}

void fsh_exec4(char ***cmd) { // executes ">" redirection 
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    
    if (pid == 0) {
        int fd = open(cmd[1][0], O_CREAT | O_TRUNC | O_WRONLY, 0660);
        if (fd == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }
        if (dup2(fd, STDOUT_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        close(fd);
        execvp(cmd[0][0], cmd[0]);
        perror("fsh");
        exit(EXIT_FAILURE);
    }
}

void fsh_exec5(char ***cmd) { // executes ">>" redirection 
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }    
    if (pid == 0) {
        int fd = open(cmd[1][0], O_WRONLY | O_APPEND | O_CREAT, 0666);
        if (fd == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }
        if (dup2(fd, STDOUT_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        close(fd);
        execvp(cmd[0][0], cmd[0]);
        perror("fsh");
        exit(EXIT_FAILURE);
    }
}

void fsh_pipeline(char ***cmd, int cmd_num) {
    int fd[cmd_num - 1][2];
    pid_t pid;
    for (int i = 0; i < cmd_num; i++){
        if (i != cmd_num - 1){
            if (pipe(fd[i]) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }
        pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            if (i != 0){
                if (dup2(fd[i - 1][0], STDIN_FILENO) == -1) {
                    perror("dup2s");
                    exit(EXIT_FAILURE);
                }
                close(fd[i - 1][0]);
            }
            if (i != cmd_num - 1){
                if (dup2(fd[i][1], STDOUT_FILENO) == -1) {
                      perror("dup2q");
                      exit(EXIT_FAILURE);
                }
                close(fd[i][0]);
                close(fd[i][1]);
            }
            execvp(cmd[i][0], cmd[i]);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
        if (i != 0){
            close(fd[i - 1][0]);
        }
        if (i != cmd_num - 1){    
            close(fd[i][1]);
        }
        wait(NULL);
    }
}

int status;

void handler(int sig) {
    wait(&status);
}

void fsh_background(char **cmd) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        signal(SIGCHLD, handler);
        signal(SIGHUP, SIG_IGN);
        execvp(cmd[0], cmd);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
}

void fsh_process(char ***cmd, char *bundle) {
    char *bundles[] = {"&&", "||", "<", ">", ">>"};
    if (strcmp(bundle, bundles[0]) == 0) {
        fsh_exec1(cmd);
        return;
    }
    if (strcmp(bundle, bundles[1]) == 0) {
        fsh_exec2(cmd);
        return;
    }
    if (strcmp(bundle, bundles[2]) == 0) {
        fsh_exec3(cmd);
        return;
    }
    if (strcmp(bundle, bundles[3]) == 0) {
        fsh_exec4(cmd);
        return;
    }
    if (strcmp(bundle, bundles[4]) == 0) {
        fsh_exec5(cmd);
        return;
    }
    printf("error\n");
    exit(EXIT_FAILURE);
}

void fsh(void) {
    char hostname[HOST_NAME_MAX];
    gethostname(hostname, HOST_NAME_MAX);
    while (!feof(stdin)) {
        printf("%s%s%s%s: ", BOLD, AC_MAGENTA, hostname, END_BOLD);
        // ignore signals
        signal(SIGINT, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);

        // read and process string from stdin
        char *line = readLine();
        if (strcmp(line, "") == 0) {
            continue;
        }
        rmExtraSpaces(line);
        rmSpace(line);
        
        // split, process and execute commands
        int cmd_num = countCommands(line);
        int start = 0;
        for (int i = 0; i < cmd_num; i++) {
            char *cmd = splitLine(line, start, ';');
            start += (strlen(cmd) + 1);
            int type = cmdHandler(cmd);
            if (type == -1) // token error
                continue;
            if (type == 0) {
                int cmd_num = 2;
                char **pcmd = parseCmd(cmd);
                char *bundle = getBundle(pcmd);
                if (bundle != NULL) {
                    char ***command = splitCommands(pcmd, cmd_num); 
                    fsh_process(command, bundle);
                    continue;
                }
                fsh_execute(pcmd);
                continue;
            }
            if (type == 1) {
                int cmd_num = countPipelineCmds(line);
                char **pcmd = parseCmd(cmd);
                char *bundle = getBundle(pcmd);
                if (bundle != NULL) {
                    char ***command = splitCommands(pcmd, cmd_num);
                    fsh_pipeline(command, cmd_num);
                    continue;
                }
            }
            if (type == 2) {
                char **pcmd = parseCmd(cmd);
                fsh_background(pcmd);
                continue;
            }
        }
    }
    putchar('\n');
}
