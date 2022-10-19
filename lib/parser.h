#ifndef PARSER_H
#define PARSER_H

void rmExtraSpaces(char *line); // removes extra spaces from buffer: ls   /home -> ls /home
void rmSpace(char *line); // removes space before and after semicolon
void addSpacesPipeline(char *line); // adds spaces for pipeline: ls|wc|cd -> ls | wc | cd
void addSpacesBgLaunch(char *str);
void addSpacesR(char *str); // adds spaces for redirections: echo text>out.txt -> echo text > out.txt
void addSpaces(char *str); // adds spaces for bundles: cmd1&&cmd2 -> cmd1 && cmd2
int cmdHandler(char *str); // handles commands and prepares it for execution
// int cmdHandler(char *str) returns -1 if error, 1 if pipeline and 0 if bundles and redirecttions
int countStrElements(char *cmd); // counts string elements (command, args and delimeters)
int countCommands(char *line); // counts num of commands
char *splitLine(char *line, int start, char delim); // splits line on substrings by delimiter
char *readLine(void); // readline from stdin and write to buffer
char **parseCmd(char *cmd);
char *getBundle(char **cmd); // returns bundle (delimiter between commands)
int *countArgs(char **cmd, int cmd_num); // returns array of num of args for each command
char ***splitCommands(char **cmd, int cmd_num); // splits commands
int countPipelineCmds(char *line); // returns num of commands in pipeline
 
#endif