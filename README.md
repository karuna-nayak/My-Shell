# My-Shell-Command-line-interpreter
Implements Shell or Command Line Interpreter (e.g. to replace /bin/bash for simple interactions with the Linux Kernel.).
The shell can execute below commands:
Execute a single command with up to four command line arguments (including command line arguments with associated flags). For example:
ls –l 
cat myfile  
ls –al /usr/src/linux

• Execute a command in background. For example:   
ls -l &  
ls –al /usr/src/linux &  

• Redirect the standard output of a command to a file. For example:   
ls -l > outfile  
ls -l >> outfile  
ls –al /usr/src/linux > outfile2  
ls –al /usr/src/linux >>outfile2  

• Redirect the standard input of a command to come from a file. For example:   
grep disk < outfile  
grep linux < outfile2  

• Execute multiple commands connected by a single shell pipe. For example:    
ls –al /usr/src/linux | grep linux  

• Execute the cd and pwd commands  
cd some_path  
pwd  

• Shell can store a history of all commands executed. 
This includes the ability to scroll through this history using up and down arrow keys as well and rerun previously entered commands.
