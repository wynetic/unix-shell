# FAKNG UNIX SHELL
#### This is C implemention of UNIX Shell.
### FSH capabilities:
**1.** sequential launch and execution of two or more commands splited by semicolon  
&nbsp;&nbsp;&nbsp;&nbsp;`ls /home/user/; echo "hello github"; screenfetch`  
**2.** execution of command bundles  
&nbsp;&nbsp;&nbsp;&nbsp;`cmd1 && cmd2`  
&nbsp;&nbsp;&nbsp;&nbsp;`cmd1 || cmd2`  
**3.** redirections  
&nbsp;&nbsp;&nbsp;&nbsp; `prog > output.txt`  
&nbsp;&nbsp;&nbsp;&nbsp; `prog >> output.txt`  
&nbsp;&nbsp;&nbsp;&nbsp; `prog < input.txt`  
**4.** pipeline  
&nbsp;&nbsp;&nbsp;&nbsp; `cmd1 | cmd2 | cmd3 | ... | cmdN`

### Built-in functions:
**1.** help  
**2.** cd  
**3.** exit  
