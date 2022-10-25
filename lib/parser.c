#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include "parser.h"

#define BUFFER_SIZE 2048
#define ARG_SIZE 256


void rmExtraSpaces(char *line) {
    int len = strlen(line);
    int i, j, quote = 0;
    for (i = 0; i < len; i++) {
        if (line[i] == '"')
            quote++;
        if (quote == 1)
            continue;
        if (quote == 2) {
            quote = 0;
            continue;
        }
        if (line[i] == ' ' && line[i + 1] == ' ') {
            for (j = i; j < (len - 1); j++) {
                line[j] = line[j + 1];
            }
            line[j] = '\0';
            len--;
            i--;
        }
    }
}

void rmSpace(char *line) {
    int len = strlen(line);
    int i, j, quote = 0;
    for (i = 0; i < len; i++) {
        if (line[i] == '"')
            quote++;
        if (quote == 1)
            continue;
        if (quote == 2) {
            quote = 0;
            continue;
        }
        if (line[i] == ';' && line[i + 1] == ' ') {
            for (j = i + 1; j < (len - 1); j++)
                line[j] = line[j + 1];
            line[j] = '\0';
        }
        if (line[i] == ';' && line[i - 1] == ' ') {
            for (j = i - 1; j < (len - 1); j++)
                line[j] = line[j + 1];
            line[j] = '\0';
        }
    }
}

void rmComments(char *str) {
    int len = strlen(str);
    int quote = 0;
    for (int i = 0; i < len; i++) {
        if (str[i] == '"')
            quote++;
        if (quote == 1)
            continue;
        if (quote == 2) {
            quote = 0;
            continue;
        }
        if (str[i] == '#' && str[i - 1] == ' ') {
            str[i] = '\0';
            for (int j = i + 1; j < len; j++) {
                str[j] = ' ';
            }
        }
    }
}

int cmdHandler(char *str) {
    int len = strlen(str);
    char delim[] = {'&', '|'};
    int quote = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < len; j++) {
            if (str[j] == delim[i] && str[j + 1] == delim[i] && str[j + 2] == delim[i]) {
                printf("syntax error near unexpected token `%c'\n", delim[i]);
                return -1; // syntax error
            }
            if (str[j] == delim[i] && str[j + 2] == delim[i]) {
                printf("syntax error near unexpected token `%c'\n", delim[i]);
                return -1; // syntax error
            }
        }
    }
    for (int i = 0; i < len; i++) {
        if (str[i] == '"')
            quote++;
        if (quote == 1)
            continue;
        if (quote == 2) {
            quote = 0;
            continue;
        }
        if (str[i] == '|' && str[i + 1] == '|' ||
            str[i] == '&' && str[i + 1] == '&') {
            addSpaces(str);
            return 1; // bundles ||, &&
        } else if (str[i] == '>' || str[i] == '<' || (str[i] == '>' && str[i + 1] == '>')) {
            addSpacesR(str);
            return 2; // redirects <, >, >>
        } else if (str[i] == '|' && str[i + 1] != '|') {
            addSpacesPipeline(str);
            return 3; // pipeline
        } else if (str[i] == '&') {
            addSpacesBgLaunch(str);
            return 4; // background launch
        }
    }
    return 0;
}

void addSpacesPipeline(char *str) {
    int len = strlen(str);
    char pipe = '|';
    for (int j = 0; j < len; j++) { // adds spaces before '|'
        if (j > 1) {
            if (str[j] == pipe && str[j - 1] != ' ') {
                str[j] = ' ';
                for (int k = len - 1; k > j; k--) {
                    str[k + 1] = str[k];
                }
                str[j + 1] = pipe;
                len++;
            }
        }
    }
    for (int j = 0; j < len; j++) { // adds spaces after '|'
        if (str[j] == pipe && str[j + 1] != ' ') {
            for (int k = len - 1; k > j; k--) {
                str[k + 1] = str[k];
            }
            str[j + 1] = ' ';
            len++;
        }
    }
}

void addSpacesR(char *str) {
    int len = strlen(str);
    char redirectors[] = {'<', '>'};
    for (int i = 0; i < strlen(redirectors); i++) {
        int f = 0;
        for (int j = 0; j < len; j++) { // adds spaces before '<' or '>'
            if ((str[j] == redirectors[i] && str[j + 1] == redirectors[i]) ||
                (str[j - 1] == redirectors[i] && str[j] == redirectors[i])) {
                f = 1;
            }
            if (f == 1 && str[j - 1] != ' ' && str[j] == redirectors[i] && str[j + 1] == redirectors[i]) {
                str[j] = ' ';
                for (int k = len - 1; k > j + 1; k--) {
                    str[k + 1] = str[k];
                }
                str[j + 2] = redirectors[i];
                len++;
                continue;
            }
            if (f == 0 && str[j] == redirectors[i] && str[j - 1] != ' ') {
                str[j] = ' ';
                for (int k = len - 1; k > j; k--) {
                    str[k + 1] = str[k];
                }
                str[j + 1] = redirectors[i];
                len++;
                continue;
            }
        }
        for (int j = 0; j < len; j++) { // adds spaces after '<' or '>'
            if ((str[j] == redirectors[i] && str[j + 1] == redirectors[i]) ||
                (str[j - 1] == redirectors[i] && str[j] == redirectors[i])) {
                f = 1;
            }
            if (f == 1 && str[j] == redirectors[i] && str[j + 1] == redirectors[i] && str[j + 2] != ' ') {
                for (int k = len - 1; k > j + 1; k--) {
                    str[k + 1] = str[k];
                }
                str[j + 2] = ' ';
                len++;
                continue;
            }
            if (f == 0 && str[j] == redirectors[i] && str[j + 1] != ' ') {
                for (int k = len - 1; k > j; k--) {
                    str[k + 1] = str[k];
                }
                str[j + 1] = ' ';
                len++;
                continue;
            }
        }
    }
}

void addSpacesBgLaunch(char *str) {
    int len = strlen(str);
    for (int j = 0; j < len; j++) { // adds spaces before '&'
        if (str[j] == '&' && str[j - 1] != ' ') {
            str[j] = ' ';
            for (int k = len - 1; k > j; k--) {
                str[k + 1] = str[k];
            }
            str[j + 1] = '&';
            len++;
        }
    }
}

void addSpaces(char *str) {
    int len = strlen(str);
    char bundles[] = {'&', '|'};
    for (int i = 0; i < strlen(bundles); i++) {
        for (int j = 0; j < len; j++) { // adds spaces before "&&" or "||"
            if (str[j - 1] != ' ' && str[j] == bundles[i] && str[j + 1] == bundles[i]) {
                str[j] = ' ';
                for (int k = len - 1; k > j + 1; k--) {
                    str[k + 1] = str[k];
                }
                str[j + 2] = bundles[i];
                len++;
            }
        }
        for (int j = 0; j < len; j++) { // adds spaces before "&&" or "||"
            if (str[j] == bundles[i] && str[j + 1] == bundles[i] && str[j + 2] != ' ') {
                for (int k = len - 1; k > j + 1; k--) {
                    str[k + 1] = str[k];
                }
                str[j + 2] = ' ';
                len++;
                continue;
            }
        }
    }
}

void write_history(char *buf) {
    char *dir = (char*) calloc(PATH_MAX, sizeof(char));
    if (dir == NULL) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    strcat(dir, getenv("HOME"));
    strcat(dir, "/.fsh_history");
    int fd = open(dir, O_CREAT | O_APPEND | O_WRONLY, 0660);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    write(fd, buf, strlen(buf));
    write(fd, "\n", sizeof(char));
    close(fd);
}

char *readLine(void) {
    int bufsize = BUFFER_SIZE;
    char *buffer = (char*) calloc(bufsize, sizeof(char));

    if (buffer == NULL) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    int pos = 0;
    int quote = 0;
    char c;
    while (1) {
        c = getchar();
        if (c == '"') {
            quote++;;
        }
        if (quote == 1 && c == '\n') {
            printf("> ");
            buffer[pos] = ' ';
            pos++;
            continue;
        }
        if (quote == 2) {
            quote = 0;
        }
        if ((c == EOF || c == '\n') &&
            ((buffer[pos - 1] == '|' && buffer[pos - 2] == '|') ||
             (buffer[pos - 1] == '&' && buffer[pos - 2] == '&') ||
             (buffer[pos - 1] == '>' && buffer[pos - 2] == '>'))) {
            printf("fsh: syntax error near unexpected token `%c%c'\n", buffer[pos - 2], buffer[pos - 1]);
            return NULL;
        }
        if ((c == EOF || c == '\n') &&
            (buffer[pos - 1] == '|' ||
            buffer[pos - 1] == '>' ||
            buffer[pos - 1] == '<')) {
            printf("fsh: syntax error near unexpected token `%c'\n", buffer[pos - 1]);
            return NULL;
        }

        if (c == EOF || c == '\n') {
            buffer[pos] = '\0';
            break;
        }
        if (pos > bufsize) {
            bufsize *= 2;
            buffer = realloc(buffer, bufsize);
            if (buffer == NULL) {
                perror("realloc");
                exit(EXIT_FAILURE);
            }
        }
        buffer[pos] = c;
        pos++;
    }
    write_history(buffer);
    return buffer;
}

int countStrElements(char *cmd) {
    int args = 1, quote = 0;
    for (int i = 0; i < strlen(cmd); i++) {
        if (cmd[i] == '"')
            quote++;
        if (quote == 1)
            continue;
        if (quote == 2) {
            quote = 0;
            continue;
        }
        if (i == 0)
            continue;
        if (cmd[i] == ' ')
            args++;
    }
    return args;
}

int countCommands(char *line) {
    int cmds = 1, quote = 0;
    for (int i = 0; i < strlen(line); i++) {
        if (line[i] == '"')
            quote++;
        if (quote == 1)
            continue;
        if (quote == 2) {
            quote = 0;
            continue;
        }
        if (line[i + 1] == '\0')
            break;
        if (line[i] == ';')
            cmds++;
    }
    return cmds;
}

int countPipelineCmds(char *line) {
    int cmdN = 1, quote = 0;
    for (int i = 0; i < strlen(line); i++) {
        if (line[i] == '"')
            quote++;
        if (quote == 1)
            continue;
        if (quote == 2) {
            quote = 0;
            continue;
        }
        if (line[i + 1] == '\0')
            break;
        if (line[i] == '|')
            cmdN++;
    }
    return cmdN;
}

char *splitLine(char *line, int start, char delim) {
    char *cmd = (char*) malloc(256 * sizeof(char));
    if (line == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    int len = strlen(line);
    int i, j = 0, quote = 0;
    for (i = start; i < len; i++) {
        if (delim == ' ') {
            if (line[i] == '"') {
                quote++;
                cmd[j] = line[i];
                j++;
                continue;
            }
            if (quote == 1) {
                cmd[j] = line[i];
                j++;
                continue;
            }
            if (quote == 2) {
                cmd[j] = line[i];
                j++;
                quote = 0;
                continue;
            }
        }
        if (line[i] == '"')
            quote++;
        if (quote == 1) {
            cmd[j] = line[i];
            j++;
            continue;
        }
        if (quote == 2) {
            cmd[j] = line[i];
            j++;
            quote = 0;
            continue;
        }

        if (line[i] == delim)
            break;
        if (line[i] == EOF || line[i] == '\0')
            break;
        cmd[j] = line[i];
        j++;
    }
    return cmd;
}

char **getSep(char **cmd) {
    char *sep[] = {"&&", "||", "|", "&", "<", ">", ">>"};
    int N = 0;
    char **seps = (char**) malloc(N * sizeof(char*));
    if (seps == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < 7; i++) {
        int j = 0;
        while (cmd[j] != NULL) {
            if (strcmp(cmd[j], sep[i]) == 0) {
                N++;
                seps = realloc(seps, N*sizeof(char*));
                if (seps == NULL) {
                    perror("realloc");
                    exit(EXIT_FAILURE);
                }
                seps[N - 1] = sep[i];
            }
            j++;
        }
    }
    return seps;
}

int *countArgs(char **cmd, int cmd_num) {
    int sep_num = countSeparators(cmd);
    char **sep = getSep(cmd);
    int *args = (int*) calloc(cmd_num, sizeof(int));
    if (args == NULL) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    for (int k = 0; k < sep_num; k++) {
        if (sep[k] != NULL) {
            int j = 0;
            for (int i = 0; i < cmd_num; i++) {
                while (cmd[j] != NULL) {
                    if (strcmp(cmd[j], sep[k]) == 0) {
                        j++;
                        break;
                    }
                    args[i]++;
                    j++;
                }
                args[i]++;
            }
        }
    }
    return args;
}

char ***splitCommands(char **cmd, int cmd_num) {
    int sep_num = countSeparators(cmd);
    char **sep = getSep(cmd);
    int *args = countArgs(cmd, cmd_num);

    char ***split = (char***) malloc(cmd_num * sizeof(char**));
    if (split == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < cmd_num; i++) {
        for (int j = 0; j < args[i]; j++) {
            split[i] = (char**) malloc(args[i] * sizeof(char*));
            if (split[i] == NULL) {
                perror("malloc");
                exit(EXIT_FAILURE);
            }
        }
    }

    int k = 0;
    for (int l = 0; l < sep_num; l++) {
        for (int i = 0; i < cmd_num; i++) {
            for (int j = 0; j < args[i]; j++) {
                if (cmd[k] == NULL) {
                    break;
                }
                if (strcmp(cmd[k], sep[l]) == 0) {
                    k++;
                    break;
                }
                split[i][j] = cmd[k];
                k++;
            }
        }
    }
    free(args);
    return split;
}

char **parseCmd(char *cmd) {
    char *sep[] = {"&&", "||", "|", "&", ">", "<", ">>"};
    int n = countStrElements(cmd);    
    int arg_size = ARG_SIZE;
    char **pcmd = (char**) malloc((n + 1)*sizeof(char*));
    if (pcmd == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    int start = 0;
    for (int i = 0; i < n; i++) {
        pcmd[i] = (char*) malloc(arg_size*sizeof(char));
        if (pcmd[i] == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        char *s = splitLine(cmd, start, ' ');
        start += (strlen(s) + 1);
        for (int j = 0; j < strlen(s); j++) {
            pcmd[i][j] = s[j];
        }
    }
    pcmd[n] = NULL;
    return pcmd;
}

char **parseCombinedIO(char **cmd) {
    int sep_num = countSeparators(cmd);
    char **sep = getSep(cmd);
    int n = sep_num + 2;
    char **io = (char**) malloc(n * sizeof(char*));
    if (io == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    int i = 0, k = 0;
    while(cmd[i] != NULL) {
        for (int j = 0; j < sep_num; j++) {
            if (strcmp(cmd[i], sep[j]) != 0) {
                io[k] = cmd[i];
                k++;
                i++;
                break;
            }
        }
        i++;
    }
    io[k] = NULL;
    return io;
}

int countSeparators(char **str) {
    char *sep[] = {"&&", "||", "|", "&", ">", "<", ">>"};
    int count = 0;
    int i = 1;
    char *s = str[0];
    while (s != NULL) {
        for (int j = 0; j < 7; j++) {
            if (strcmp(s, sep[j]) == 0) {
                count++;
                i++;
                break;
            }
        }
        s = str[i];
        i++;
    }
    return count;
}

int checkSeparators(char **sep, int sep_num) {
    char *s = sep[0];
    for (int i = 1; i < sep_num; i++) {
        if (strcmp(s, sep[i]) != 0) {
            printf("fsh: it's not available\n");
            return -1;
        }
    }
    return 0;
}
