#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <sys/wait.h>

#define BSIZE 256
#define MAX_JOBS 100

struct job {
    int quashID = 0; // quashID for job, will be the job's value + 1
    int pid; // actual system PID for the job
    char command[BSIZE]; // string storing the command that started the command
    char formatted[BSIZE]; // string storing the formatted version of the job showing the quashID, PID, and command line
} jobList[MAX_JOBS];

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
int backExe(char** exe, char* unparsed, char* output) { // takes in executable name and arguments as a string. Also takes in unparsed to be passed into job list and takes in output that will be printed or passed somewhere else
    //should be similar to forExe but need to hide the process in the background
    //maybe pipes that don't wait for it to return so that other things can happen?
    pid_t p = fork(); // calls fork on pid p

    if (p == 0) { // child process
        for (int i = 0; i < MAX_JOBS; i++) { // iterates through indices of jobs array
            if (jobList[i].quashID == 0) { // checks if the current quashID value is 0 meaning that it is empty
                char jobbuf[BSIZE]; // creates a buffer for the new job being added to the list
                bzero(jobbuf, BSIZE); // empties the buffer
                sprintf(jobbuf, "[%d] %d %s", i+1, getpid(), unparsed); // Adds job to buffer in format [QUASH PID] PID COMMAND
                jobList[i].quashID = i+1; // sets the quashID as the index + 1
                jobList[i].pid = getpid(); // sets the pid variable as its pid
                strcpy(jobList[i].command, unparsed); // adds the command to the command variable
                strcpy(jobList[i].formatted, jobbuf); // adds the formatted text of the new job to the formatted variable
                printf("Background job started: %s", jobbuf); // prints that the job started with its information
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
int echoString(char* parsed, int numberOfItems) { // takes in string to print (needs to remove "echo" work from start of string)
    // maybe need different method than direct printing?
    for (int i = 1; i < numberOfItems; i++) { // iterates through parsed starting at index 1 to not print out the echo command word
        if (strcmp(parsed[i][0], "$") == 0) { // checks if environmental variable
            printf("%s ", getenv(parsed[i])); // gets environmental variable value and prints
        }
        printf("%s ", parsed[i]); // prints if anything else
    }
    printf("\n"); // prints new line
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

// List Directory Contents - ls
int ls(char* arg) { // takes in single letter argument
    //need to find list of arguments it takes and implement each one
}

// Commands that are built in with key words
int builtInCmds(char* input) { // takes in parsed input. Returns 0 for success, 1 for no matching command, and 2 for incorrect parameters for matching command
    int cmdType = -1; // initializes as -1 to verify if none of the commands are matches
    char* allCmds[8]; // creates an array that will store names of commands to verify which one was passed in
    
    allCmds[0] = "echo";
    allCmds[1] = "export";
    allCmds[2] = "jobs";
    allCmds[3] = "quit";
    allCmds[4] = "exit";
    allCmds[5] = "cd";
    allCmds[6] = "pwd";
    allCmds[7] = "ls";
    allCmds[8] = "kill";

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
                return 2; // returns 2 to signify error in input parameters
            }
            return 0;

        case 6: // Print Path of Current Directory - pwd (Simple command)
            char directorybuf[BSIZE]; // creates a buffer for the current working directory
            bzero(directorybuf, BSIZE); // empties the buffer
            getcwd(directorybuf, BSIZE); // gets the current working directory and stores it in the buffer
            printf("%s", directorybuf); // prints the buffer which stores the current directory

        case 7: // List Directory Contents - ls
            ls(input[1]); // calls ls with the first index value which is the letter parameter

        case 8:// Send POSIX Signal to Process - kill (Simple command)
            if (kill(input[2], input[1]) == -1) {
                return 2;
            } // VERIFY SIGNIAL IS SENT SOMEHOW??
            return 0;
    }
}

void parser(char *input, char parsed[5][256]) { { // takes in string to parse
    //start at front of string and iterate through and store each character in a new buffer
    //stop when hitting one of the special characters or end of line
    //if stopped from end of line then check if any of the commands that don't take in any parameters
    //if it doesn't match any of the commands then call forExe on it to run it as an executable
    //if stopped from special character then wait till finished parsing
    //continue reading if more characters after special character and store the second half in different buffer
    //run error message if not enough or too many parameters are given instead of passing in function and finding an error
    //each function should have independent error handling if incorrect parameters are passed, parser should just check quantity and type
    int inLen = strlen(input); 
    int single = 0, dub = 0, count = 0;
    
    
    for (int j = 0; j < inLen; j++)  
    { 
        
        if(input[j] == '\'' && dub == 0)
        {
            if (count == 0)
            { 
                single = 1; 
                count++;
            }
            else 
            { 
                single = 0; 
                count--;
            }
        }
        
        if(input[j] == '\"' && single == 0)
        {
            if (count == 0)
            { 
                dub = 1; 
                count++;
            }
            else 
            { 
                dub = 0; 
                count--;
            } 
        }
        
        if(single == 0 && dub == 0)
        {
            if (input[j] == '#')
            {
                input[j] = '\0';
                break;
            }
            
            if (input[j] == '=') 
            {
                input[j] = ' '; 
            }
        }
    }
            
    int i = 0;
    char *temp;
    while ((temp = strsep(&input, " ")) != NULL) {
        strcpy(parsed[i], temp);  
        i++;
    }

    for (int j = 0; j < i; j++) {
        printf("%s\n", parsed[j]);
    }

}
    
    
}

int parseThenPass(char* input) { // parses input and runs corresponding command/executable
    char* parsed[BSIZE]; // creates an array that will store the tokenized input from parser function
    int midline = parser(input, parsed); // calls parser and stores the return value to check if pipes or redirection exist in the input
    
    switch(midline) { // switch block to check if the parser needs to be called again for pipe or redirect
        case 0: // runs if there is no midline modifier
            int builtIn = builtInCmds(parsed); // calls builtInCmds and stores return to check if success, no match, or error
            switch(builtIn) { // switch block to check if a command was success, no match, or error
                case 0: // runs if built in command was matched and successful
                    break; // no other actions

                case 1: // runs if no built in command was matched
                    if (strcmp(parsed[sizeof(parsed[0])], "&") == 0) { // checks if the last item of the array is an & meaning it needs to run in the background
                        backExe(parsed, input); // executes the file in the background if possible
                    } else {
                        forExe(parsed); // executes the file in the foreground
                    }
                    break;

                case 2: // runs if an error occured with one of the built in commands
                    printf("Invalid syntax for '%s' command\n", parsed[0]); // states there was an error and what the attempted command was
                    break;

            }
            break;

        case 1: // Pipes - | (Midline Modifier)

        case 2: // Input Redirection - < (Midline Modifier)

        case 3: // Output Redirection - > (Midline Modifier)
        // fork inside here to redirect output to something else and then call backExe within that.
        // this will cause a background parent to wait for the background process but the main process with keep going if it's supposed to be ran in the background

        case 4: // Redirect Output While Appending Output - >> (Midline Modifier)

    }

}




// Comments - # (Midline Modifier)

// Bonus
// Pipes and Redirects Can Be Mixed
// Pipes and Redirects Work with Built In Commands

int main() {
    char* jobs[]; // creates array to store the running jobs
    printf("Welcome...\n");
    while(1) {
        for (int i = 0; i < MAX_JOBS; i++) { // iterates through jobList
            if (jobList[i].quashID != 0) { // checks if the current quashID is not 0, meaning it is storing a process
                if (kill(jobList[i].pid, 0) == -1) { // sends a 0 signal to the PID which does nothing but will return -1 if it fails, meaning the process isn't running
                    printf("Completed: %s", jobList[i].formatted); // prints that the process completed
                    jobList[i].quashID = 0; // sets the quashID to 0 to signify the process ended
                }
            }
        }
        //waits for user input at the start of each loop
        //takes the input and passes it to parser which utilizes it from there
        //error handling in here?
    }
}