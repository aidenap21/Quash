#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h> 
#include <sys/wait.h> 

#define BSIZE 256
#define MAX_JOBS 100

struct job {
    int quashID; // quashID for job, will be the job's value + 1
    int pid; // actual system PID for the job
    char command[BSIZE]; // string storing the command that started the command
    char formatted[BSIZE]; // string storing the formatted version of the job showing the quashID, PID, and command line
} jobList[MAX_JOBS];

// Foreground Executables (In Progress) need to figure out how to redirect output to the output buffer
int forExe(char exe[][BSIZE], int numberOfItems) { // takes in executable name and arguments as a string. Also takes in output that will be printed or passed somewhere else
    int status;
    char *exePtr[BSIZE];  // array of pointers to strings

    for (int i = 0; i < numberOfItems; i++) { // iterates through all values
        exePtr[i] = exe[i];  // sets the pointer values to the exe value
    }

    exePtr[numberOfItems] = NULL; // makes value after all arguments NULL

    /*
    for (int i = 0; i < numberOfItems + 1; i++) {
        printf("exe: %s\n", exe[i]);
        printf("exePtr: %s\n", &exePtr[i]);
    }
    printf("HERE\n");
    */
    
    pid_t p = fork(); // calls fork on pid p

    if (p == 0) { // child process
        if (execvp(exe[0], exePtr) < 0) { // calls execvp on passed in executable with unparsed as parameters but catches error if exec fails
            printf("Error executing...\n"); // prints statement that exec fails
            exit(0); // exits child process since it failed
        }

    } else if (p > 0) { // parent process
        if ((waitpid(p, &status, 0)) == -1) {
            fprintf(stderr, "Process encountered error...");
  } // parent waits for child to finish executing
    } else {
        printf("Fork failed...\n");
    }

    return 0;
}

// Background Executables - & (In Progress)  need to figure out how to redirect output to the output buffer and properly add to job list
int backExe(char exe[][BSIZE], char* unparsed, int numberOfItems) { // takes in executable name and arguments as a string. Also takes in unparsed to be passed into job list and takes in output that will be printed or passed somewhere else
    //should be similar to forExe but need to hide the process in the background
    //maybe pipes that don't wait for it to return so that other things can happen?
    char *exePtr[BSIZE];  // array of pointers to strings

    for (int i = 0; i < numberOfItems - 1; i++) { // goes to numberOfItems - 1 to remove &
        exePtr[i] = exe[i];  // sets the pointer values to the exe value
    }

    exePtr[numberOfItems - 1] = NULL; // makes value after all arguments NULL

    pid_t p = fork(); // calls fork on pid p

    if (p == 0) { // child process
        int i;
        for (i = 0; i < MAX_JOBS; i++) { // iterates through indices of jobs array
            if (jobList[i].quashID == 0) { // checks if the current quashID value is 0 meaning that it is empty
                char jobbuf[BSIZE]; // creates a buffer for the new job being added to the list
                bzero(jobbuf, BSIZE); // empties the buffer
                sprintf(jobbuf, "[%d] %d %s", i+1, getpid(), unparsed); // Adds job to buffer in format [QUASH PID] PID COMMAND
                jobList[i].quashID = i+1; // sets the quashID as the index + 1
                jobList[i].pid = getpid(); // sets the pid variable as its pid
                strcpy(jobList[i].command, unparsed); // adds the command to the command variable
                strcpy(jobList[i].formatted, jobbuf); // adds the formatted text of the new job to the formatted variable
                printf("Background job started: %s\n", jobbuf); // prints that the job started with its information
                break; // ends loop because space was found in jobList
            }
        }
        if (execvp(exe[0], exePtr) < 0) { // calls execvp on passed in executable with parameters but catches error if exec fails
            printf("Error executing background process...\n"); // prints statement that exec fails
            printf("Background job removed due to error: %s\n", jobList[i].formatted);
            jobList[i].quashID = 0;
            exit(0); // exits child process since it failed 
        }

    } else if (p < 0) {
        printf("Fork failed...");
    }

    return 0;
}


// Print String - echo (In Progress)
int echoString(char parsed[][BSIZE], int numberOfItems, char* output) { // takes in string to print (needs to remove "echo" work from start of string)
    char currentItem[BSIZE]; // will store word of current iteration
    int nextIsVar = 0; // flag to check if value following $ is environmental variable
    bzero(output, BSIZE); // empties the buffer

    for (int i = 1; i < numberOfItems; i++) { // iterates through parsed starting at index 1 to not print out the echo command word
        bzero(currentItem, BSIZE); // empties the buffer
        if (parsed[i][0] == '$') { // checks if environmental variable
            nextIsVar = 1; // marks flag

        } else if (nextIsVar == 1) { // runs if next item is variable
            sprintf(currentItem, "%s", getenv(parsed[i])); // gets environmental variable value and adds to output
            strcat(output, currentItem); // concatenates the current item to the output
            nextIsVar = 0; // resets flag

        } else { // runs if next item is just text
            sprintf(currentItem, "%s ", parsed[i]); // adds to output
            strcat(output, currentItem); // concatenates the current item to the output
        }
    }
    strcat(output, "\n"); // adds new line to output
    bzero(currentItem, BSIZE); // empties the buffer
}

// Set Value of Environmental Variable - export
int export(char parsed[][BSIZE], int numberOfItems) {
    int variable, value, found = 0, otherVar = 0; // variable and value store the indices of the two and found is a check to see if variable is found to then find value
    for (int i = 1; i < numberOfItems; i++) { // iterates through number of items
        if (parsed[i][0] == '$' && found == 1) { // if $ and variable is already found it runs
            otherVar = 1; // sets value to 1 to know that it will be getting variable value from another environmental variable
            continue; // continues since $ isn't being used directly
        }
        if (found == 0) { // runs if variable name isn't found yet
            variable = i; // stores the index of the variable
            found = 1; // sets the found check to 1 to show variable has been found
        } else { // runs if value hasn't been found yet
            value = i; // stores the index of the value
            found = 2;
            break;
        }
    }
    if (found != 2) { // runs if not enough parameters were passed through
        return -1; // returns -1 to signify error
    } else if (otherVar == 1){ // runs if the value being used is another environmental variable
        return setenv(parsed[variable], getenv(parsed[value]), 1); // gets the value of the value and sets the variable to that, returns 0 for success and -1 for failure
    } else {
        return setenv(parsed[variable], parsed[value], 1); // sets the variable to the value, returns 0 for success and -1 for failure
    }
}

// Print All Running Background Process - jobs
void printJobs() {
    //need an array that is storing all currently running jobs
    //iterate through the job array and print
    printf("Print Jobs Called\n");
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

    for (int i = 0; i < 8; i++) { // iterates through the command array
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

            if (export(parsed, numberOfItems) == -1) { // calls export function
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
            printf("%s\n", directorybuf); // prints the buffer which stores the current directory
            return 0; // returns 0 to signify success

        case 7:// Send POSIX Signal to Process - kill (Simple command)
            if (kill(atoi(parsed[2]), atoi(parsed[1])) == -1) { // calls kill with PID and then signal from input after they've been converted to ints
                return 2; // returns 2 to signify error in calling kill
            } 
            return 0; // returns 0 to signify success
    }
}

int parser(char *input, char parsed[BSIZE][BSIZE], char leftover[BSIZE], int *pLen)  //parameters for input string and matrix to output the parsed data to
{
    int inLen = strlen(input), curStart = 0, curWord = 0; 
    
    if(inLen == 0) //check if the input is empty
    { return 5; }
    
    int single = 0, dub = 0, count = 0, environs = 0; //single to check for '', dub to check for "", count to see if there is already a " or ' in the command
    
    for (int j = 0; j < inLen; j++) //Loop thru entire input
    { 
        if(input[j] == ' ' && dub == 0 && single == 0) { // checks if current character is space and isn't in quotes
            strncpy(parsed[curWord], input + curStart, j - curStart); // truncates the current word and stores it
            curStart = j + 1; // resets the start value for the next word
            curWord++; // increments the next available word position
        }

        if(input[j] == '|' && dub == 0 && single == 0) { // runs if on | and not in quotes
            strncpy(leftover, input + j, inLen - j + 1); // truncates the remainder after the midline modifier and stores it
            leftover[j - curStart + 1] = '\0';
            *pLen = curWord;
            return 1; // returns 1 to show | was found
        }

        if(input[j] == '<' && dub == 0 && single == 0) { // runs if on < and not in quotes
            strncpy(leftover, input + j, inLen - j + 1); // truncates the remainder after the midline modifier and stores it
            leftover[j - curStart + 1] = '\0';
            *pLen = curWord;
            return 2; // returns 2 to show < was found
        }

        if(input[j] == '>' && dub == 0 && single == 0) { // runs if on < and not in quotes
            if(input[j+1] == '>') { // runs if next character is also > for >>
                strncpy(leftover, input + j + 1, inLen - j + 2); // truncates the remainder after the midline modifier and stores it
                leftover[j - curStart + 1] = '\0';
                *pLen = curWord;
                return 4; // returns 4 to show >> was found

            } else {
                strncpy(leftover, input + j, inLen - j + 1); // truncates the remainder after the midline modifier and stores it
                leftover[j - curStart + 1] = '\0';
                *pLen = curWord;
                return 3; // returns 3 to show > was found
            }
        }

        if(input[j] == '$' && dub == 0 && single == 0)
        {
            parsed[curWord][0] = '$'; // sets value to $ separate from following variable name itself
            curWord++; // increments curWord to use the next position
            curStart = j + 1; // resets curStart
            environs = 1; // sets environs to 1 as flag to stop when / is found
        }
        
        if(input[j] == '/' && dub == 0 && single == 0)
        {
            strncpy(parsed[curWord], input + curStart, j - curStart); // truncates the current word and stores it
            curStart = j; // resets the start value for the next word but uses / as start instead of next character
            curWord++; // increments the next available word position
            environs = 0;
        }

        if(input[j] == '\'' && dub == 0 && environs == 0) //Case for ''
        {
            if (count == 0) // beginning quote
            { 
                single = 1; 
                count++;
            }
            else // ending quote
            { 
                strncpy(parsed[curWord], input + curStart + 1, j - curStart - 1); // adds extra 1 to start to remove quote characters
                curStart = j + 1; // resets the start value for the next word
                curWord++; // increments the next available word position
                single = 0; 
                count--;
            }
        }
        
        if(input[j] == '\"' && single == 0 && environs == 0) //Case for ""
        {
            if (count == 0) // beginning quote
            { 
                dub = 1; 
                count++;
            }
            else // ending quote
            { 
                strncpy(parsed[curWord], input + curStart + 1, j - curStart - 1); // adds extra 1 to start to remove quote characters
                curStart = j + 1; // resets the start value for the next word
                curWord++; // increments the next available word position
                dub = 0; 
                count--;
            } 
        }
        
        if(single == 0 && dub == 0 && environs == 0) //Case for when the input is not surrounded by quotes
        {
            if (input[j] == '#') //Check for comments
            {
                if(j == 0) //Return if there is a comment at the very start
                { 
                    *pLen = curWord;
                    return 5;
                } 
                
                input[j] = '\0'; //End the readable code after comment
                break;
            }
            
            if (input[j] == '=') //Replace any '=' with spaces
            {
                strncpy(parsed[curWord], input + curStart, j - curStart); // truncates the current word and stores it
                curStart = j + 1; // resets the start value for the next word
                curWord++; // increments the next available word position 
            }
        }
    }
    strncpy(parsed[curWord], input + curStart, inLen - curStart); // truncates the current word and stores it
    curWord++;
    *pLen = curWord; // sets number of items as current word number
    return 0; // returns 0 to signify no midline modifiers
}

void parseThenPass(char* input) { // parses input and runs corresponding command/executable
    char parsed[BSIZE][BSIZE]; // creates an array that will store the tokenized input from parser function
    char leftover[BSIZE], outputBuf[BSIZE];
    int numberOfItems = 0;
    int midline = parser(input, parsed, leftover, &numberOfItems); // calls parser and stores the return value to check if pipes or redirection exist in the input

    

    switch(midline) { // switch block to check if the parser needs to be called again for pipe or redirect
        case 0: ;// runs if there is no midline modifier
            int builtIn = builtInCmds(parsed, numberOfItems, outputBuf); // calls builtInCmds and stores return to check if success, no match, or error
            switch(builtIn) { // switch block to check if a command was success, no match, or error
                case 0: // runs if built in command was matched and successful
                    printf("%s", outputBuf); // prints the outputBuf which may store the echo output or be empty
                    break; // no other actions

                case 1: // runs if no built in command was matched
                    if (parsed[numberOfItems - 1][0] == '&') { // checks if the last item of the array is an & meaning it needs to run in the background
                        backExe(parsed, input, numberOfItems); // executes the file in the background if possible
                    } else {
                        forExe(parsed, numberOfItems); // executes the file in the foreground
                    }
                    break;

                case 2: // runs if an error occured with one of the built in commands
                    printf("Invalid syntax for '%s' command\n", parsed[0]); // states there was an error and what the attempted command was
                    break;

            }
            break;

        case 1: // Pipes - | (Midline Modifier)
            break;

        case 2: // Input Redirection - < (Midline Modifier)
            break;

        case 3: // Output Redirection - > (Midline Modifier)
        // fork inside here to redirect output to something else and then call backExe within that.
        // this will cause a background parent to wait for the background process but the main process with keep going if it's supposed to be ran in the background
            break;
        case 4: // Redirect Output While Appending Output - >> (Midline Modifier)
            break;

    }
    for(int i = 0; i < BSIZE; i++) {
        bzero(parsed[i], BSIZE);
    }
    bzero(leftover, BSIZE);
    bzero(outputBuf, BSIZE);
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
        printf("[QUASH]$ "); // prints line
        scanf(" %[^\n]%*c", input); // gets input from user
        parseThenPass(input); 
        //waits for user input at the start of each loop
        //takes the input and passes it to parser which utilizes it from there
        //error handling in here?
    }

    return 0;
}