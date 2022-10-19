#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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

int cmdHandler(char *str) {
    int len = strlen(str);
    char delim[] = {'&', '|'};
    int quote = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < len; j++) {
            if (str[j] == delim[i] && str[j + 1] == delim[i] && str[j + 2] == delim[i]) {
                printf("syntax error near unexpected token `%c`\n", delim[i]);
                return -1;
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
        if ((str[i] == '|' && str[i + 1] == '|') || 
            (str[i] == '&' && str[i + 1] == '&') ||
            (str[i] == '>' && str[i + 1] == '>')) { // bundle and redirect out \w append
            addSpaces(str);
            return 0;
        } else if (str[i] == '|' && str[i + 1] != '|') { // pipeline
            addSpacesPipeline(str);
            return 1;
        } else if (str[i] == '>' || str[i] == '<') { // redirections
            addSpacesR(str);
            return 0;
        } else if (str[i] == '&') {
            addSpacesBgLaunch(str);
            return 2;
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
    char redirerctions[] = {'<', '>'};
    for (int i = 0; i < strlen(redirerctions); i++) {
        for (int j = 0; j < len; j++) { // adds spaces before '<' or '>'
            if (str[j] == redirerctions[i] && str[j - 1] != ' ') {
                str[j] = ' ';
                for (int k = len - 1; k > j; k--) {
                    str[k + 1] = str[k];
                }
                str[j + 1] = redirerctions[i];
                len++;
            }
        }
        for (int j = 0; j < len; j++) { // adds spaces after '<' or '>'
            if (str[j] == redirerctions[i] && str[j + 1] != ' ') {
                for (int k = len - 1; k > j; k--) {
                    str[k + 1] = str[k];
                }
                str[j + 1] = ' ';
                len++;
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
    char bundles[] = {'&', '|', '>'};
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

char *readLine(void) {
    int bufsize = BUFFER_SIZE;
    char *buffer = (char*) calloc(bufsize, sizeof(char));

    if (buffer == NULL) {
        fprintf(stderr, "malloc: error\n");
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
        if (c == EOF || c == '\n') {
            buffer[pos] = '\0';
            break;
        }
        if (pos > bufsize) {
            bufsize *= 2;
            buffer = realloc(buffer, bufsize);
        }
        buffer[pos] = c;
        pos++;
    }
    return buffer;
}

int countStrElements(char *cmd) {
    int args = 0, quote = 0;
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
        if (line[i] == '|')
            cmds++;
    }
    return cmds;
}

char *splitLine(char *line, int start, char delim) {
    char *cmd = (char*) malloc(256 * sizeof(char));
    if (line == NULL) {
        fprintf(stderr, "memory error\n");
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

char *getBundle(char **cmd) {
    char *bundles[] = {"&&", "||", "|", "&", "<", ">", ">>"};
    for (int i = 0; i < 7; i++) {
        int j = 0;
        while (cmd[j] != NULL) {
            if (strcmp(cmd[j], bundles[i]) == 0) {
                return bundles[i];
                break;
            }
            j++;
        }
    }
    return NULL;
}

int *countArgs(char **cmd, int cmd_num) {
    char *bundle = getBundle(cmd);
    int *args = (int*) calloc(cmd_num, sizeof(int));
    if (bundle != NULL) {
        int j = 0;
        for (int i = 0; i < cmd_num; i++) {
            while (cmd[j] != NULL) {
                if (strcmp(cmd[j], bundle) == 0) {
                    j++;
                    break;
                }
                args[i]++;
                j++;
            }
            args[i]++;
        }
    }
    return args;
}

char ***splitCommands(char **cmd, int cmd_num) {
    char *bundle = getBundle(cmd);
    int *args = countArgs(cmd, cmd_num);

    char ***split = (char***) malloc(cmd_num * sizeof(char**));
    for (int i = 0; i < cmd_num; i++) {
        for (int j = 0; j < args[i]; j++) {
            split[i] = (char**) malloc(args[i] * sizeof(char*));
        }
    }

    int k = 0;
    for (int i = 0; i < cmd_num; i++) {
        for (int j = 0; j < args[i]; j++) {
            if (cmd[k] == NULL) {
                break;
            }
            if (strcmp(cmd[k], bundle) == 0) {
                k++;
                break;
            }
            split[i][j] = cmd[k];
            k++;
        }
    }
    free(args);
    return split;
}

char **parseCmd(char *cmd) {
    int n = countStrElements(cmd) + 1;
    int arg_size = ARG_SIZE;
    char **pcmd = (char**) malloc((n + 1)*sizeof(char*));
    int start = 0;
    for (int i = 0; i < n; i++) {
        pcmd[i] = (char*) malloc(arg_size*sizeof(char));
        char *s = splitLine(cmd, start, ' ');
        start += (strlen(s) + 1);
        for (int j = 0; j < strlen(s); j++) {
            pcmd[i][j] = s[j];
        }
    }
    pcmd[n] = NULL;
    return pcmd;
}
