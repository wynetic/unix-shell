#ifndef PARSER_H
#define PARSER_H


void freeDoublePointer(char **ptr);
void rmExtraSpaces(char *line); // removes extra spaces from buffer: ls   /home -> ls /home
// void rmSpace(char *line); // removes space before and after semicolon
// void rmComments(char *str); // replaces symbols after '#' by spaces
void addSpacesPipeline(char *line); // adds spaces for pipeline: ls|wc|cd -> ls | wc | cd
void addSpacesBgLaunch(char *str); // adds spaces before &
void addSpacesR(char *str); // adds spaces for redirections: echo text>out.txt -> echo text > out.txt
void addSpaces(char *str); // adds spaces for bundles: cmd1&&cmd2 -> cmd1 && cmd2
int cmdHandler(char *str); // handles commands and prepares it for execution
// int cmdHandler(char *str) returns -1 if error, 1 if pipeline and 0 if bundles and redirecttions
int countStrElements(char *cmd); // counts string elements (command, args and delimeters)
int countCommands(char *line); // counts num of commands
char *splitLine(char *line, int start, char delim); // splits line on substrings by delimiter
void write_history(char *buf); // write command to ~/.fsh_history
char *readLine(void); // readline from stdin and write to buffer
int syntaxChecker(char *s);
char **parseCmd(char *cmd);
char **getSep(char **cmd); // returns bundles (delimiters between commands)
int *countArgs(char **cmd, int cmd_num); // returns array of num of args for each command
char ***splitCommands(char **cmd, int cmd_num); // splits commands
int countPipelineCmds(char *line); // returns num of commands in pipeline
int countSeparators(char **str); // return num of separators
int checkSeparators(char **sep, int sep_num); // check if all delimiters are the same
char **parseCombinedIO(char **cmd); // parse combined IO `prog < in.txt > out.txt`

#endif