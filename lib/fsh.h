#ifndef FSH_H
#define FSH_H

// ======[built-in functions ]======
void fsh_exit(void);
void fsh_help(void);
void fsh_cd(char **dir);
// ======[ exec functions ]======
void fsh_echo(char **cmd); // echo "abc    bcd" -> abc    bcd (without quotes)
void fsh_exec1(char ***cmd); // executes "&&" bundle
void fsh_exec2(char ***cmd); // executes "||" bundle
void fsh_exec3(char ***cmd); // executes "<" redirection
void fsh_exec4(char ***cmd); // executes ">" redirection
void fsh_exec5(char ***cmd); // executes ">>" redirection
void fsh_pipeline(char ***cmd, int cmd_num); // exec pipeline
void handler(int sig);
void fsh_background(char **cmd); // run command in the background
void fsh_process(char ***cmd, char *bundle); // processes splitted command and executes bundle
void fsh_execute(char **cmd); // executes commands
void fsh(void); // main function to start shell

#endif