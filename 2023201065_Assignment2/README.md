# Assignment 2
## Question 1
### Overview
1. The execution of terminal begins from main(). The .cpp file contains all the function defined, but all the function calls to the respective function are made in main() and these function calls depend on the user input.
2. The main() conatains a while loop which runs contionusly so as to mimic the behaviour of system terminal.
3. the tokeniser() takes user input and converts to tokens of string.(the delimiter here is blank space) and the return a vector to main function. This vector will contain all the token of strings.
4. Now the first token of the vector will always be a command and using if-else if- else statement the main() will compare with conditions and if a conditon matches, will call the coresponding function.
5. Now below this function I will give a brief overview of functions that have been implemented by me.
6. displayOnScreen(): The function is responsible for giving the details about username, hostname and the current directory in which the terminal is working. All these details are printed on the screen. Using basic calls like getenv("USER"), gethostname(hostname, _SC_HOST_NAME_MAX+1) the details of current user is fetched. This function also takes a parameter of current working directory (cwd) so that the new directory is updated on terminal whenever cd command is used.
7. changeDir(): It is reponsible to change the directory based on the user input.
8. echo(): It prints everything written after the echo command. It appends all the token(except echo word) to a string prints it on screen. The function can also write to all data written after "echo" command to an mentioned  output file by calling outputRedirector().
9. cat_the_file(): The function prints the content of the user input file on the screen. Also it can copy the contents of input file to output file when the user explictly mentions using ">" or ">>". It checks the existance of input file and if it exists then it opens the file using open() reads it using read() and writes it on screen if ">" or ">>" is not given in input else it will write to output file.
10. lsFunctionNormal: which takes a parameter of file path is responsible to display all non hidden files and directories in the current directory on the screen. Triggered by ls command.
11. lsFunctionA(): which takes a parameter of file path is responsible to display all the hidden as well as non hidden files and directories in the current directory on the screen. Triggered by ls -a command.
12. lsFunctionL(): which takes a parameter of file path is responsible to display the vital metadata of non hidden files. Uses stat function to fetch the metadata. It call printPermissions() to print the file permission of the file or directory on the screen. Triggered by ls -l command.
13. The commands ls -l -a, ls -a -l, ls -al, ls -la all triggers lsFunctionL() which lists all the metadata of both hidden as well as non hidden file.
14. The command ls -a file/directory name list all the metadata of hidden as well as non hidden file of file/directory name while ls -l file/directory name list all the metadata of non hidden files.
15. ls .. list all the files of parent directory.
16. ls ~ list all the files of home directory.
17. ls . list all the files of current directory.
18. Search(): which takes an parameter of file name and scans for the file or directory in the current directory. If found displays True on screen else False.

### Execution.
```c++
chmod +x 2023201065_A2.cpp
``` 
```c++
./2023201065_A2.cpp
``` 

### Assumptions

1. That user should give input.
2. Home directory will be same as the home directory of the sytem and that my terminal will also start from home directory just like system terminal.
3. In changeDir() the directory will not change if the path given is relative and the current directory and the irectory to which the user want to go are at same level. If the directory does not exist the an error message will be displayed on screen.
4. Search(): Will scans only the current directory and no other directory. The file name should be given by user.



