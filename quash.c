#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <sys/wait.h>

#define BSIZE 256
#define MAX_JOBS 10

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
int ls(char* arg) { // takes in single letter argument
    //need to find list of arguments it takes and implement each one
}

// Foreground Executables (In Progress) need to figure out how to redirect output to the output buffer
int forExe(char** exe, char* output) { // takes in executable name and arguments as a string. Also takes in output that will be printed or passed somewhere else
    pid_t p = fork(); // calls fork on pid p

    if (p == 0) { // child process
        if (execvp(exe[0], exe) < 0) { // calls execvp on passed in executable with parameters but catches error if exec fails
            printf("Error executing..."); // prints statement that exec fails
            return 0; 
        }

    } else { // parent process 
        wait(NULL); // parent waits for child to finish executing
    }

    return 0;
}

// Background Executables - & (In Progress)  need to figure out how to redirect output to the output buffer and properly add to job list
int backExe(char** exe, char* output, char* jobs) { // takes in executable name and arguments as a string. Also takes in output that will be printed or passed somewhere else and job list
    //should be similar to forExe but need to hide the process in the background
    //maybe pipes that don't wait for it to return so that other things can happen?
    pid_t p = fork(); // calls fork on pid p

    if (p == 0) { // child process
        for (int i = 0; i < MAX_JOBS; i++) { // iterates through indices of jobs array
            if (jobs[i] == NULL) { // checks if the current index value is NULL to add the new job to it
                char jobbuf[BSIZE]; // creates a buffer for the new job being added to the list
                bzero(jobbuf, BSIZE); // empties the buffer
                sprintf(jobbuf, "[%d] %d %s", i+1, getpid(), exe); // Adds job to buffer in format [QUASH PID] PID COMMAND
                jobs[i] = jobbuf; // adds the new job to the jobs array DON'T THINK THIS SYNTAX IS RIGHT
            }
        }
        if (execvp(exe[0], exe) < 0) { // calls execvp on passed in executable with parameters but catches error if exec fails
            printf("Error executing..."); // prints statement that exec fails
            return 0; 
        }

    } else { // parent process
        
    }

    return 0;
}


// Print String - echo (In Progress)
int echoString(char* string) { // takes in string to print (needs to remove "echo" work from start of string)
    // maybe need different method than direct printing?
    printf("%s\n", string); // prints string and new line
}

// Set Value of Environmental Variable - export
int export(char *var, char *val) { // takes in variable name and value to update with (not positive about variable type) returns 0 on success and 1 on failure
    //need to know what variables need to actually be defined that are going to be updated
    //check var and compare to variable names to figure out what to update
}


// Print All Running Background Process - jobs
int printJobs() {
    //need an array that is storing all currently running jobs
    //iterate through the job array and print

}

// Commands that are built in with key words
int buildInCmds(char* input) { // takes in parsed input. Returns 0 for success, 1 for no matching command, and 2 for incorrect parameters for matching command
    int cmdType = -1; // initializes as -1 to verify if none of the commands are matches
    char* allCmds[8]; // creates an array that will store names of commands to verify which one was passed in
    
    allCmds[0] = "echo";
    allCmds[1] = "export";
    allCmds[2] = "jobs";
    allCmds[3] = "quit";
    allCmds[4] = "exit";
    allCmds[5] = "cd";
    allCmds[6] = "pwd";
    allCmds[7] = "kill";

    for (int i = 0; i++; i < 8) { // iterates through the command array
        if (strcmp(input[0], allCmds[i]) == 0) { // compares the first index of input to the current command to see if it matches
            cmdType = i; // sets the switch value number if it matches
            break; // breaks the loop since the match was already found
        } 
    }
    switch(cmdType) {
        case -1: // Not valid simple command
            return 1; // returns 1 to signify no matching commands

        case 0: // Print String - echo (In Progress)
            echoString(input); // calls echoString function
            return 0; // returns 0 to signify success

        case 1: // Set Value of Environmental Variable - export
            if (export(input[1], input[2]) == 1) { // passed in first index as variable name and second as value // USE TRY EXCEPT TO VERIFY VALID INPUT WITH NUMBER OF ITEMS IN INPUT ARRAY
                return 2; // returns 2 to signify error in input parameters
            }
            return 0; // returns 0 to signify success

        case 2: // Print All Running Background Process - jobs
            printJobs(); // calls printJobs function
            return 0; // returns 0 to signify success

        case 3: // Terminate Quash - quit (Simple command)
            exit(0); // ends the program

        case 4: // Terminate Quash - exit (Simple command)
            exit(0); // ends the program

        case 5: // Change Working Directory - cd (Simple command)
            if (chdir(input[1]) == -1) { // this may be wrong, need to verify what chdir returns if unsuccessful //VERIFY WITH TRY EXCEPT FOR INPUT
                printf("Invalid directory..."); // state that the directory is invalid
                return 2; // returns 2 to signify error in input parameters
            }
            return 0;

        case 6:// Print Path of Current Directory - pwd (Simple command)

        case 7:// Send POSIX Signal to Process - kill (Simple command)
            kill(input[1], input[2]); // VERIFY SIGNIAL IS SENT SOMEHOW??
            return 0;
    }
}

int parser(char* input) { // takes in string to parse
    //start at front of string and iterate through and store each character in a new buffer
    //stop when hitting one of the special characters or end of line
    //if stopped from end of line then check if any of the commands that don't take in any parameters
    //if it doesn't match any of the commands then call forExe on it to run it as an executable
    //if stopped from special character then wait till finished parsing
    //continue reading if more characters after special character and store the second half in different buffer
    //run error message if not enough or too many parameters are given instead of passing in function and finding an error
    //each function should have independent error handling if incorrect parameters are passed, parser should just check quantity and type

}

// Input Redirection - < (Midline Modifier)
// Output Redirection - > (Midline Modifier)
// Redirect Output While Appending Output - >> (Midline Modifier)
// Pipes - | (Midline Modifier)
// Comments - # (Midline Modifier)

// Bonus
// Pipes and Redirects Can Be Mixed
// Pipes and Redirects Work with Built In Commands

int main() {
    char* jobs[]; // creates array to store the running jobs
    printf("Welcome...\n");
    while(1) {
        //waits for user input at the start of each loop
        //takes the input and passes it to parser which utilizes it from there
        //error handling in here?
    }
}