#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <sys/wait.h>

#define BSIZE 256

//these are bash specified bins
//maybe make different file for each function and run it instead of in 1 big file?
//shouldn't use bash directories and should build our own instead I think
#define BASH_EXEC  "/bin/bash"
#define FIND_EXEC  "/bin/find"
#define XARGS_EXEC "/usr/bin/xargs"
#define GREP_EXEC  "/bin/grep"
#define SORT_EXEC  "/bin/sort"
#define HEAD_EXEC  "/usr/bin/head"



// List Directory Contents - ls
int ls(char arg) { // takes in single letter argument
    //need to find list of arguments it takes and implement each one
}

// Foreground Executables (In Progress)
int forExe(char *exe, char *arg, char *output) { // takes in executable name and arguments as a string. Also takes in output that will be printed or passed somewhere else
    int status;
    pid_t p = fork();

    if(p == 0) {
        char cmdbuf[BSIZE];
        bzero(cmdbuf, BSIZE);
        sprintf(cmdbuf, "%s %s", exe, arg);

        //exec call needs to change since BASH_EXEC isn't going to be there
        if ((execl(BASH_EXEC, BASH_EXEC, "-c", cmdbuf, (char *) 0)) < 0) {
            fprintf(stderr, "\nError execing bash. ERROR#%d\n", errno);
            return EXIT_FAILURE;
        }
    }

    if ((waitpid(p, &status, 0)) == -1) {
        fprintf(stderr, "Process 1 encountered an error. ERROR%d", errno);
        return EXIT_FAILURE;
    }

    return 0;
}

// Background Executables - &
int backExe(char *exe, char *arg) { // takes in executable name and arguments as a string. Also takes in output that will be printed or passed somewhere else
    //should be similar to forExe but need to hide the process in the background
    //maybe pipes that don't wait for it to return so that other things can happen?
}


// Print String - echo (In Progress)
int echoString(char *string) { // takes in string to print
    // maybe need different method than direct printing?
    printf("%s", string); 
}

// Set Value of Environmental Variable - export
//int export(char *var, char *val) { // takes in variable name and value to update with (not positive about variable type)
    //need to know what variables need to actually be defined that are going to be updated
    //check var and compare to variable names to figure out what to update
//}


// Print All Running Background Process - jobs
int printJobs() {
    //need an array that is storing all currently running jobs
    //iterate through the job array and print

}

int parser(char *input) { // takes in string to parse
    //start at front of string and iterate through and store each character in a new buffer
    //stop when hitting one of the special characters or end of line
    //if stopped from end of line then check if any of the commands that don't take in any parameters
    //if it doesn't match any of the commands then call forExe on it to run it as an executable
    //if stopped from special character then wait till finished parsing
    //continue reading if more characters after special character and store the second half in different buffer
    //run error message if not enough or too many parameters are given instead of passing in function and finding an error
    //each function should have independent error handling if incorrect parameters are passed, parser should just check quantity and type

}
// Input Redirection - < (Build into parser)
// Output Redirection - > (Build into parser)
// Redirect Output While Appending Output - >> (Build into parser)
// Pipes - | (Build into parser)
// Comments - # (Build into parser)
// Terminate Quash - quit, exit (Build into parser)
// Change Working Directory - (Build into parser)
// Print Path of Current Directory - pwd (Build into parser)
// Send POSIX Signal to Process - kill (Build into parser)


// Bonus
// Pipes and Redirects Can Be Mixed
// Pipes and Redirects Work with Built//In Commands

int main() {
    printf("Welcome...\n");
    while(1) {
        //waits for user input at the start of each loop
        //takes the input and passes it to parser which utilizes it from there
        //error handling in here?
    }
}