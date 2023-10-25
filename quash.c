#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>

#define BSIZE 256
#define MAX_JOBS 100

struct job {
    int quashID; // quashID for job, will be the job's value + 1
    int pid; // actual system PID for the job
    char command[BSIZE]; // string storing the command that started the command
    char formatted[BSIZE]; // string storing the formatted version of the job showing the quashID, PID, and command line
} jobList[MAX_JOBS];

// Foreground Executables (In Progress) need to figure out how to redirect output to the output buffer
int forExe(char exe[][BSIZE]) { // takes in executable name and arguments as a string. Also takes in output that will be printed or passed somewhere else
    pid_t p = fork(); // calls fork on pid p

    if (p == 0) { // child process
        if (execvp(exe[0], exe) < 0) { // calls execvp on passed in executable with parameters but catches error if exec fails
            printf("Error executing..."); // prints statement that exec fails
            return 0; 
        }

    } else { // parent process 
        wait(); // parent waits for child to finish executing
    }

    return 0;
}

// Background Executables - & (In Progress)  need to figure out how to redirect output to the output buffer and properly add to job list
int backExe(char exe[][BSIZE], char* unparsed) { // takes in executable name and arguments as a string. Also takes in unparsed to be passed into job list and takes in output that will be printed or passed somewhere else
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
int echoString(char parsed[][BSIZE], int numberOfItems, char* output) { // takes in string to print (needs to remove "echo" work from start of string)
    // maybe need different method than direct printing?
    char currentItem[BSIZE];
    for (int i = 2; i < numberOfItems; i++) { // iterates through parsed starting at index 1 to not print out the echo command word
        bzero(currentItem, BSIZE);
        if (strcmp(parsed[i][0], "$") == 0) { // checks if environmental variable
            sprintf(currentItem, "%s ", getenv(parsed[i])); // gets environmental variable value and adds to output
        }
        sprintf(currentItem, "%s ", parsed[i]); // adds to output
        strcat(output, currentItem);
    }
    strcat(output, "\n"); // adds new line to output
}

/*
// Set Value of Environmental Variable - export
int export(char* parsed) { // takes in variable name and value to update with (not positive about variable type) returns 0 on success and 1 on failure
    //need to know what variables need to actually be defined that are going to be updated
    //check var and compare to variable names to figure out what to update
    setenv
}
*/


// Print All Running Background Process - jobs
void printJobs() {
    //need an array that is storing all currently running jobs
    //iterate through the job array and print
    for (int i = 0; i < MAX_JOBS; i++) { // iterates through jobList
        if (jobList[i].quashID != 0) { // checks if the current quashID is not 0, meaning it is storing a process
            printf("%s\n", jobList[i].formatted); // prints the process
        }
    }

}

// Commands that are built in with key words
int builtInCmds(char parsed[][BSIZE], int numberOfItems, char* output) { // takes in parsed input. Returns 0 for success, 1 for no matching command, and 2 for incorrect parameters for matching command
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
        if (strcmp(parsed[0], allCmds[i]) == 0) { // compares the first index of input to the current command to see if it matches
            cmdType = i; // sets the switch value number if it matches
            break; // breaks the loop since the match was already found
        } 
    }
    switch(cmdType) {
        case -1: // Not valid simple command
            return 1; // returns 1 to signify no matching commands

        case 0: // Print String - echo (In Progress)
            echoString(parsed, numberOfItems, output); // calls echoString function
            return 0; // returns 0 to signify success

        case 1: // Set Value of Environmental Variable - export
            if (setenv(parsed[1], parsed[2]) == -1) { // passed in first index as variable name and second as value // USE TRY EXCEPT TO VERIFY VALID INPUT WITH NUMBER OF ITEMS IN INPUT ARRAY
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
            if (chdir(parsed[1]) == -1) { // this may be wrong, need to verify what chdir returns if unsuccessful //VERIFY WITH TRY EXCEPT FOR INPUT
                return 2; // returns 2 to signify error in calling cd
            }
            return 0;

        case 6: ;// Print Path of Current Directory - pwd (Simple command)
            char directorybuf[BSIZE]; // creates a buffer for the current working directory
            bzero(directorybuf, BSIZE); // empties the buffer
            getcwd(directorybuf, BSIZE); // gets the current working directory and stores it in the buffer
            printf("%s", directorybuf); // prints the buffer which stores the current directory

        case 7:// Send POSIX Signal to Process - kill (Simple command)
            if (kill(parsed[2], parsed[1]) == -1) { // calls kill with PID and then signal from input
                return 2; // returns 2 to signify error in calling kill
            } 
            return 0; // returns 0 to signify success
    }
}

    //void parser(char *input, char parsed[5][256]) {  // takes in string to parse
    //start at front of string and iterate through and store each character in a new buffer
    //stop when hitting one of the special characters or end of line
    //if stopped from end of line then check if any of the commands that don't take in any parameters
    //if it doesn't match any of the commands then call forExe on it to run it as an executable
    //if stopped from special character then wait till finished parsing
    //continue reading if more characters after special character and store the second half in different buffer
    //run error message if not enough or too many parameters are given instead of passing in function and finding an error
    //each function should have independent error handling if incorrect parameters are passed, parser should just check quantity and type

int parser(char *input, char parsed[256][256], char leftover[256], int *pLen)  //parameters for input string and matrix to output the parsed data to
{
    int inLen = strlen(input); 
    
    if(inLen == 0) //check if the input is empty
    { return 5; }
    
    int single = 0, dub = 0, count = 0; //single to check for '', dub to check for "", count to see if there is already a " or ' in the command
    
    for (int j = 0; j < inLen; j++) //Loop thru entire input
    { 
        if(input[j] == '\'' && dub == 0) //Case for ''
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
        
        if(input[j] == '\"' && single == 0) //Case for ""
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
        
        if(single == 0 && dub == 0) //Case for when the input is not surrounded by quotes
        {
            if (input[j] == '#') //Check for comments
            {
                if(j == 0) //Return if there is a comment at the very start
                { return 5; } 
                
                input[j] = '\0'; //End the readable code after comment
                break;
            }
            
            if (input[j] == '=') //Replace any '=' with spaces
            {
                input[j] = ' '; 
            }
        }
    }
            
    int i = 0; 
    char *temp; //Temp buffer to hold the current space-separated item
    while ((temp = strsep(&input, " ")) != NULL) //Separate everything by spaces
    {
        strncpy(leftover, input + i-2, inLen); //Copy the current word into the parsed array
        
        int opType = -1;
        
        char* operations[4];
        
        operations[0] = "|";
        operations[1] = "<";
        operations[2] = ">";
        operations[3] = ">>";
        

        for(int j = 0; j<4; j++) 
        {
            if (strcmp(temp, operations[j]) == 0)
            {
                opType = j;
                break;
            }
        }
        
        if(opType != -1) //Check if there is an operation that needs to be handled
        {
            switch(opType)
            {
                case 0: // pipe case
                    strncpy(leftover, input + i-2, inLen);
                    *pLen = inLen - *input +i;
                    return 1;
                    
                case 1: // < case
                    strncpy(leftover, input + i-2, inLen);
                    *pLen = inLen - *input +i;
                    return 2;
                
                case 2: // > case
                    strncpy(leftover, input + i-2, inLen);
                    *pLen = inLen - *input +i;
                    return 3;
                    
                case 3: // >> case
                    strncpy(leftover, input + i-2, inLen);
                    *pLen = inLen - *input +i;
                    return 4;
            }
        }
        i++;
    }
    *pLen = inLen;
    return 0;

}
    
    


int parseThenPass(char* input) { // parses input and runs corresponding command/executable
    char parsed[BSIZE][BSIZE]; // creates an array that will store the tokenized input from parser function
    char leftover[BSIZE], outputBuf[BSIZE];
    bzero(leftover, BSIZE);
    bzero(outputBuf, BSIZE);
    int numberOfItems = 0;
    int midline = parser(input, parsed, leftover, numberOfItems); // calls parser and stores the return value to check if pipes or redirection exist in the input
    
    switch(midline) { // switch block to check if the parser needs to be called again for pipe or redirect
        case 0: ;// runs if there is no midline modifier
            int builtIn = builtInCmds(parsed, numberOfItems, outputBuf); // calls builtInCmds and stores return to check if success, no match, or error
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
    for (int i = 0; i < MAX_JOBS; i++) { // iterates through all the jobs
        jobList[i].quashID = 0; // sets all jobs to quashID of 0 to indicate it's clear
    }
    char input[BSIZE]; // creates a character buffer to store input from the user
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
        bzero(input, BSIZE); // empties the buffer
        printf("[QUASH]$ ");
        gets(input);
        printf("\n");
        parseThenPass(input);
        //waits for user input at the start of each loop
        //takes the input and passes it to parser which utilizes it from there
        //error handling in here?
    }
}