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
#define AC_BLUE "\033[0;34m"
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
    puts("");
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

void fsh_exec1(char ***cmd, int cmd_num) { // executes "&&" bundle
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
    for (int i = 1; i < cmd_num; i++) {
        wait(&status);
        if (WIFEXITED(status)) {
            if (!WEXITSTATUS(status)) {
                pid = fork();
                if (pid == -1) {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                if (pid == 0) {
                    execvp(cmd[i][0], cmd[i]);
                    perror("fsh");
                    exit(EXIT_FAILURE);
                }
                wait(NULL);
            }
        }
    }
}

void fsh_exec2(char ***cmd, int cmd_num) { // executes "||" bundle
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
    for (int i = 1; i < cmd_num; i++) {
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
}

void fsh_exec3(char ***cmd) { // executes "<" redirect
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

void fsh_exec4(char ***cmd) { // executes ">" redirect 
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
    wait(NULL);
}

void fsh_exec5(char ***cmd) { // executes ">>" redirect 
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
    wait(NULL);
}

void fsh_exec6(char **cmd) { // exec <, then >
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        int fd1 = open(cmd[1], O_EXCL | O_RDONLY); // для инпута
        if (fd1 == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }
        if (dup2(fd1, STDIN_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        close(fd1);
        int fd2 = open(cmd[2], O_CREAT | O_TRUNC | O_WRONLY, 0660); // для записи оутпута
        if (fd2 == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }
        if (dup2(fd2, STDOUT_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        close(fd2);
        execvp(cmd[0], cmd);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    wait(NULL);
}

void fsh_exec7(char **cmd) { // exec <, then >>
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        int fd1 = open(cmd[1], O_EXCL | O_RDONLY); // для инпута
        if (fd1 == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }
        if (dup2(fd1, STDIN_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        close(fd1);
        int fd2 = open(cmd[2], O_WRONLY | O_APPEND | O_CREAT, 0666); // для записи оутпута
        if (fd2 == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }
        if (dup2(fd2, STDOUT_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        close(fd2);
        execvp(cmd[0], cmd);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    wait(NULL);
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

void fsh_process(char ***cmd, int cmd_num, char *sep) {
    char *separators[] = {"&&", "||"};
    if (strcmp(sep, separators[0]) == 0) {
        fsh_exec1(cmd, cmd_num);
        return;
    }
    if (strcmp(sep, separators[1]) == 0) {
        fsh_exec2(cmd, cmd_num);
        return;
    }
    printf("fsh: error (&& or ||)\n");
    exit(EXIT_FAILURE);
}

void fsh_processIO(char ***cmd, char *sep) {
    char *redirectors[] = {"<", ">", ">>"};
    if (strcmp(sep, redirectors[0]) == 0) {
        fsh_exec3(cmd);
        return;
    }
    if (strcmp(sep, redirectors[1]) == 0) {
        fsh_exec4(cmd);
        return;
    }
    if (strcmp(sep, redirectors[2]) == 0) {
        fsh_exec5(cmd);
        return;
    }
    printf("fsh: error (redirects)\n");
    exit(EXIT_FAILURE);
}

void fsh_processCIO(char **cmd, char **sep) {
    if (strcmp(sep[0], "<") == 0 && strcmp(sep[1], ">") == 0) {
        fsh_exec6(cmd);
        return;
    } else if (strcmp(sep[0], "<") == 0 && strcmp(sep[1], ">>") == 0) {
        fsh_exec7(cmd);
        return;
    } else {
        printf("fsh: syntax error (combined redirects)\n");
        return;
    }
}

void fsh(void) {
    // get hostname
    char hostname[HOST_NAME_MAX];
    gethostname(hostname, HOST_NAME_MAX);
    // change default dir to /home/user
    char dir[PATH_MAX];
    strcat(dir, "/home/");
    strcat(dir, getenv("USER"));
    if (chdir(dir) == -1) {
        perror("cd");
        exit(EXIT_FAILURE);
    }
    
    while (!feof(stdin)) {
        char cwd[PATH_MAX];
        getcwd(cwd, PATH_MAX);
        printf("%s%s%s@%s:", BOLD, AC_MAGENTA, getenv("USER"), hostname); // print username and hostname
        printf("%s%s%s$ ", AC_BLUE, cwd, END_BOLD); // print cwd
        // ignore signals
        signal(SIGINT, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);

        // read and process string from stdin
        char *line = readLine();
        if (line == NULL) {
            continue;
        }
        if (strcmp(line, "") == 0) {
            continue;
        }
        rmComments(line);
        rmExtraSpaces(line);
        rmSpace(line);
        
        // split, process and execute commands
        int cmd_num = countCommands(line);
        int start = 0;
        for (int i = 0; i < cmd_num; i++) {
            char *cmd = splitLine(line, start, ';');
            start += (strlen(cmd) + 1);
            int type = cmdHandler(cmd);
            if (type == -1) // syntax error
                continue;
            if (type == 0) {
                char **pcmd = parseCmd(cmd);
                fsh_execute(pcmd);
                continue;
            }
            if (type == 1) {
                char **pcmd = parseCmd(cmd);
                int sep_num = countSeparators(pcmd);
                int n = sep_num + 1;
                char **sep = getSep(pcmd);
                if (checkSeparators(sep, sep_num) == -1) {
                    free(sep);
                    continue;
                }
                if (sep[0] != NULL) {
                    char ***command = splitCommands(pcmd, n); 
                    fsh_process(command, n, sep[0]);
                    free(sep);
                    continue;
                }
            } else if (type == 2) {
                char **pcmd = parseCmd(cmd);
                int n = countSeparators(pcmd);
                char **sep = getSep(pcmd);
                if (n == 1) {
                    if (sep[0] != NULL) {
                        char ***command = splitCommands(pcmd, n + 1); 
                        fsh_processIO(command, sep[0]);
                        free(sep);
                        continue;
                    }
                } else if (n == 2) {
                    char **io = parseCombinedIO(pcmd);
                    fsh_processCIO(io, sep);
                    free(sep);
                    continue;
                } else {
                    printf("fsh: too many arguments\n");
                    continue;
                }
            } else if (type == 3) {
                int n = countPipelineCmds(line);
                char **pcmd = parseCmd(cmd);
                int sep_num = countSeparators(pcmd);
                char **sep = getSep(pcmd);
                if (checkSeparators(sep, sep_num) == -1) {
                    free(sep);
                    continue;
                }
                if (sep[0] != NULL) {
                    char ***command = splitCommands(pcmd, n);
                    fsh_pipeline(command, n);
                    free(sep);
                    continue;
                }
            } else if (type == 4) {
                char **pcmd = parseCmd(cmd);
                fsh_background(pcmd);
                continue;
            }
        }
        free(line);
    }
    putchar('\n');
}
