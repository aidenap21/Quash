#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
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
} jobList[MAX_JOBS + 1]; // makes it greater than max jobs to store QUASH main PID in end

// Foreground Executables (In Progress) need to figure out how to redirect output to the output buffer
int forExe(char exe[][BSIZE], int numberOfItems) { // takes in executable name and arguments as a string. Also takes in output that will be printed or passed somewhere else
    char currentItem[BSIZE]; // will store word of current iteration
    int i, curIndex = 0, nextIsVar = 0, nextIsOutFile = 0, status; // holds current index of exePtr, flag to check if next item is $, status of child
    char *exePtr[BSIZE];  // array of pointers to strings

    for (i = 0; i < numberOfItems; i++) { // iterates through parsed starting at index 1 to not print out the echo command word
        bzero(currentItem, BSIZE); // empties the buffer
        if (exe[i][0] == '$') { // checks if environmental variable
            nextIsVar = 1; // marks flag

        } else if (exe[i][0] == '<') { // runs if input symbol
            continue; // continues to not add it to the array

        } else if (strcmp(exe[i], ">>") == 0) { // checks if append output cae is next
            nextIsOutFile = 2; // marks flag for next iteration
            break;

        } else if (exe[i][0] == '>') { // checks if output case is next
            nextIsOutFile = 1; // marks flag for next iteration
            break;

        } else if (nextIsVar == 1) { // runs if next item is variable
            sprintf(currentItem, "%s", getenv(exe[i])); // gets environmental variable value and adds to output
            exePtr[curIndex] = currentItem; // stores the converted environmental variable
            curIndex++; // increments curIndex
            nextIsVar = 0; // resets flag

        } else { // runs if next item is just text
            exePtr[curIndex] = exe[i]; // sets the next index of exePtr to the i index of exe
            curIndex++; // increments curIndex
        }
    }

    exePtr[curIndex] = NULL; // sets null to show end of args
    
    pid_t p = fork(); // calls fork on pid p

    if (p == 0) { // child process
        if (nextIsOutFile == 1) { // runs if > was found
            FILE *outputFile;
            outputFile = freopen(exe[i+1], "w", stdout);
        } else if (nextIsOutFile == 2) { // runs if >> was found
            FILE *outputFile;
            outputFile = freopen(exe[i+1], "a", stdout);
        }
        //printf("CHILD PID IN FOREXE: %d\n", getpid());
        if (execvp(exe[0], exePtr) < 0) { // calls execvp on passed in executable with unparsed as parameters but catches error if exec fails
            printf("Error executing...\n"); // prints statement that exec fails
            exit(0); // exits child process since it failed
        }

    } else if (p < 0) { // parent process
        printf("Fork failed...\n");
    }
    
    if ((waitpid(p, &status, 0)) == -1) {
            fprintf(stderr, "Process encountered error...\n");
        } // parent waits for child to finish executing
    //printf("PARENT FINISHED WAITING\n");
    

    return 0;
}

// Background Executables - & (In Progress)  need to figure out how to redirect output to the output buffer and properly add to job list
int backExe(char exe[][BSIZE], char* unparsed, int numberOfItems) { // takes in executable name and arguments as a string. Also takes in unparsed to be passed into job list and takes in output that will be printed or passed somewhere else
    //should be similar to forExe but need to hide the process in the background
    //maybe pipes that don't wait for it to return so that other things can happen?
    char currentItem[BSIZE]; // will store word of current iteration
    int i, curIndex = 0, nextIsVar = 0, nextIsOutFile = 0; // holds current index of exePtr, flag to check if next item is $
    char *exePtr[BSIZE];  // array of pointers to strings

    for (i = 0; i < numberOfItems - 1; i++) { // iterates through parsed starting at index 1 to not print out the echo command word or &
        bzero(currentItem, BSIZE); // empties the buffer
        if (exe[i][0] == '$') { // checks if environmental variable
            nextIsVar = 1; // marks flag

        } else if (exe[i][0] == '<') { // runs if input symbol
            continue; // continues to not add it to the array

        } else if (strcmp(exe[i], ">>") == 0) { // checks if append output cae is next
            nextIsOutFile = 2; // marks flag for next iteration
            break;

        } else if (exe[i][0] == '>') { // checks if output case is next
            nextIsOutFile = 1; // marks flag for next iteration
            break;

        } else if (nextIsVar == 1) { // runs if next item is variable
            sprintf(currentItem, "%s", getenv(exe[i])); // gets environmental variable value and adds to output
            exePtr[curIndex] = currentItem; // stores the converted environmental variable
            curIndex++; // increments curIndex
            nextIsVar = 0; // resets flag

        } else { // runs if next item is just text
            exePtr[curIndex] = exe[i]; // sets the next index of exePtr to the i index of exe
            curIndex++; // increments curIndex
        }
    }

    exePtr[curIndex] = NULL; // sets null to show end of args

    pid_t p = fork(); // calls fork on pid p

    if (p == 0) { // child process
        if (nextIsOutFile == 1) {
            FILE *outputFile;
            outputFile = freopen(exe[i+1], "w", stdout);
        } else if (nextIsOutFile == 2) {
            FILE *outputFile;
            outputFile = freopen(exe[i+1], "a", stdout);
        }

        if (execvp(exe[0], exePtr) < 0) { // calls execvp on passed in executable with parameters but catches error if exec fails
            printf("Error executing background process...\n"); // prints statement that exec fails
            exit(0); // exits child process since it failed 
        }

    } else if (p > 0) { // parent process
        for (int i = (MAX_JOBS - 1); i >= 0; i--) { // iterates through indices of jobs array
            if (jobList[i].quashID != 0 || i == 0) { // checks if the current quashID value is 1 meaning that it is not empty, sets next value after as new job
                char jobbuf[BSIZE]; // creates a buffer for the new job being added to the list
                bzero(jobbuf, BSIZE); // empties the buffer
                sprintf(jobbuf, "[%d] %d %s", i+1, p, unparsed); // Adds job to buffer in format [QUASH ID] PID COMMAND
                
                jobList[i+1].quashID = i+1; // sets the quashID as the index + 1
                jobList[i+1].pid = p; // sets the pid variable as the pid of the child
                strcpy(jobList[i+1].command, unparsed); // adds the command to the command variable
                strcpy(jobList[i+1].formatted, jobbuf); // adds the formatted text of the new job to the formatted variable
                printf("Background job started: %s\n", jobbuf); // prints that the job started with its information
                bzero(jobbuf, BSIZE); // empties the buffer
                break; // ends loop because space was found in jobList
            }
        }

    } else { // runs if fork fails
        printf("Fork failed...\n");
    }

    return 0;
}

void pipeExe(char exe[][BSIZE], char* leftover, int numberOfItems) {
    char currentItem[BSIZE]; // will store word of current iteration
    int curIndex = 0, nextIsVar = 0, status, p1[2]; // holds current index of exePtr, flag to check if next item is $, status, pipe
    char *exePtr[BSIZE];  // array of pointers to strings
    pid_t pid1, pid2;

    for (int i = 0; i < numberOfItems; i++) { // iterates through parsed starting at index 1 to not print out the echo command word
        bzero(currentItem, BSIZE); // empties the buffer
        if (exe[i][0] == '$') { // checks if environmental variable
            nextIsVar = 1; // marks flag

        } else if (exe[i][0] == '<') { // runs if input symbol
            continue; // continues to not add it to the array

        } else if (nextIsVar == 1) { // runs if next item is variable
            sprintf(currentItem, "%s", getenv(exe[i])); // gets environmental variable value and adds to output
            exePtr[curIndex] = currentItem; // stores the converted environmental variable
            curIndex++; // increments curIndex
            nextIsVar = 0; // resets flag

        } else { // runs if next item is just text
            exePtr[curIndex] = exe[i]; // sets the next index of exePtr to the i index of exe
            curIndex++; // increments curIndex
        }
    }

    exePtr[curIndex] = NULL; // sets null to show end of args
    
    pipe(p1);

    pid1 = fork(); // calls fork on pid p
    if (pid1 == 0) { // first child process
        printf("PID OF CHILD 1: %d\n", getpid());
        close(p1[0]);
        dup2(p1[1], 1);
        close(p1[1]);

        if (execvp(exe[0], exePtr) < 0) { // calls execvp on passed in executable with unparsed as parameters but catches error if exec fails
            printf("Error executing...\n"); // prints statement that exec fails
            exit(0); // exits child process since it failed
        }
    } else if (pid1 < 1) {
        printf("Fork failed...\n");
    }

    pid2 = fork(); // calls fork on pid p
    if (pid2 == 0) { // second child process
        printf("PID OF CHILD 2: %d\n", getpid());
        close(p1[1]);
        dup2(p1[0], 0);
        close(p1[0]);

        parseThenPass(leftover);
        exit(0);
    } else if (pid2 < 1) {
        printf("Fork failed...\n");
    }

    if ((waitpid(pid1, &status, 0)) == -1) {
        fprintf(stderr, "Process encountered error...\n");
    }
    printf("PARENT FINISHED WAITING IN PIPE FOR PID1: %d\n", pid1);

    if ((waitpid(pid2, &status, 0)) == -1) {
        fprintf(stderr, "Process encountered error...\n");
    }
    printf("PARENT FINISHED WAITING IN PIPE FOR PID2: %d\n", pid2);
}

// Print String - echo (In Progress)
int echoString(char parsed[][BSIZE], int numberOfItems) { // takes in string to print (needs to remove "echo" work from start of string)
    char currentItem[BSIZE], output[BSIZE]; // will store word of current iteration
    int nextIsVar = 0, nextIsInFile = 0, nextIsOutFile = 0; // flag to check if value following $ is environmental variable
    bzero(output, BSIZE); // empties the buffer

    for (int i = 1; i < numberOfItems; i++) { // iterates through parsed starting at index 1 to not print out the echo command word
        bzero(currentItem, BSIZE); // empties the buffer
        if (parsed[i][0] == '$') { // checks if environmental variable
            nextIsVar = 1; // marks flag for next iteration

        } else if (strcmp(parsed[i], ">>") == 0) { // checks if append output cae is next
            nextIsOutFile = 2; // marks flag for next iteration

        } else if (parsed[i][0] == '>') { // checks if output case is next
            nextIsOutFile = 1; // marks flag for next iteration

        } else if (parsed[i][0] == '<') { // checks if input case is next
            nextIsInFile = 1; // marks flag for next iteration

        } else if (nextIsVar == 1) { // runs if item is variable
            sprintf(currentItem, "%s", getenv(parsed[i])); // gets environmental variable value and adds to output
            strcat(output, currentItem); // concatenates the current item to the output
            nextIsVar = 0; // resets flag

        } else if (nextIsInFile == 1) { // runs if item is file
            FILE *inputFile; // creates file type
            inputFile = fopen(parsed[i], "r"); // opens file in read mode
            fgets(currentItem, BSIZE, inputFile); // reads from the file and stores it in the buffer
            strcat(output, currentItem); // concatenates the current item to the output
            fclose(inputFile); // closes the file
            break; 

        } else if (nextIsOutFile == 1) { // runs if item is file
            //strcat(output, "\n"); // adds new line to output
            FILE *outputFile; // creates file type
            outputFile = fopen(parsed[i], "w"); // opens file in write mode
            fprintf(outputFile, output); // writes the output to the file
            fclose(outputFile); // closes the file
            break;

        } else if (nextIsOutFile == 2) { // runs if item is file
            //strcat(output, "\n"); // adds new line to output
            sprintf(currentItem, "\n%s", output); // adds new line to the output and stores it in current item
            FILE *outputFile; // creates file type
            outputFile = fopen(parsed[i], "a"); // opens file in append mode
            fprintf(outputFile, currentItem); // writes the output to the file
            fclose(outputFile); // closes the file
            break;

        } else { // runs if next item is just text
            sprintf(currentItem, "%s ", parsed[i]); // adds to output
            strcat(output, currentItem); // concatenates the current item to the output
        }
    }

    if (nextIsOutFile == 0) { // runs if the it didn't output to a file
        printf("%s\n", output); // prints the output
    }
    bzero(currentItem, BSIZE); // empties the buffer
    bzero(output, BSIZE); // empties the buffer
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
    for (int i = 0; i < MAX_JOBS; i++) { // iterates through jobList
        if (jobList[i].quashID != 0) { // checks if the current quashID is not 0, meaning it is storing a process
            printf("%s\n", jobList[i].formatted); // prints the process
        }
    }
}

// Commands that are built in with key words
int builtInCmds(char parsed[][BSIZE], int numberOfItems) { // takes in parsed input. Returns 0 for success, 1 for no matching command, and 2 for incorrect parameters for matching command
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
            echoString(parsed, numberOfItems); // calls echoString function
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
            usleep(1000); // waits to make sure the process receives the signal before the next iteration
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
            strncpy(leftover, input + j + 2, inLen - j + 2); // truncates the remainder after the midline modifier and stores it
            //leftover[j - curStart + 1] = '\0';
            *pLen = curWord;
            return 1; // returns 1 to show | was found
        }
        
        /*
        if(input[j] == '<' && dub == 0 && single == 0) { // runs if on < and not in quotes
            parsed[curWord][0] = '<'; // sets value to $ separate from following variable name itself
            curWord++; // increments curWord to use the next position
            curStart = j + 2; // resets curStart
        }
        

         OLD VERSION OF HANDLING <
        if(input[j] == '<' && dub == 0 && single == 0) { // runs if on < and not in quotes
            strncpy(leftover, input + j + 2, inLen - j + 2); // truncates the remainder after the midline modifier and stores it
            leftover[j - curStart + 1] = '\0';
            *pLen = curWord;
            return 2; // returns 2 to show < was found
        }
        

        if(input[j] == '>' && dub == 0 && single == 0) { // runs if on < and not in quotes
            if(input[j+1] == '>') { // runs if next character is also > for >>
                strncpy(leftover, input + j + 3, inLen - j + 2); // truncates the remainder after the midline modifier and stores it
                leftover[j - curStart + 1] = '\0';
                *pLen = curWord;
                return 3; // returns 4 to show >> was found

            } else {
                strncpy(leftover, input + j + 2, inLen - j + 2); // truncates the remainder after the midline modifier and stores it
                leftover[j - curStart + 1] = '\0';
                *pLen = curWord;
                return 2; // returns 3 to show > was found
            }
        }
        */

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
    //printf("parseThenPass call\n");
    char parsed[BSIZE][BSIZE]; // creates an array that will store the tokenized input from parser function
    char leftover[BSIZE];
    int numberOfItems = 0, background = 1; // creates variable to store number of parsed items and if there is an & at the end for midline modifiers
    int midline = parser(input, parsed, leftover, &numberOfItems); // calls parser and stores the return value to check if pipes or redirection exist in the input

    /*
    for(int i = 0; i < numberOfItems; i++) { // prints each parsed item
        printf("item %d: {%s}\n", i, parsed[i]);
    }
    */
    //printf("midline flag: %d, leftover: %s\n", midline, leftover);

    switch(midline) { // switch block to check if the parser needs to be called again for pipe or redirect
        case 0: ;// runs if there is no midline modifier
            int builtIn = builtInCmds(parsed, numberOfItems); // calls builtInCmds and stores return to check if success, no match, or error
            switch(builtIn) { // switch block to check if a command was success, no match, or error
                case 0: // runs if built in command was matched and successful
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
            // both foreground and background use the same function
            // fork outside of function call before using
            // wait pid if foreground but don't wait if background so that shell continues
            // need to add check if first function isn't an executable and is built in instead
            //printf("Pipe Call\n");
            int status;
            pid_t pid = fork();
            if (pid == 0) {
                printf("PIPE HERE CALLED WITH PID: %d\n", getpid());
                pipeExe(parsed, leftover, numberOfItems);
                exit(0);
            } else if (pid > 0) {
                if (background == 0) {
                    for (int i = (MAX_JOBS - 1); i >= 0; i--) { // iterates through indices of jobs array
                        if (jobList[i].quashID != 0 || i == 0) { // checks if the current quashID value is 1 meaning that it is not empty, sets next value after as new job
                            char jobbuf[BSIZE]; // creates a buffer for the new job being added to the list
                            bzero(jobbuf, BSIZE); // empties the buffer
                            sprintf(jobbuf, "[%d] %d %s", i+1, pid, input); // Adds job to buffer in format [QUASH ID] PID COMMAND
                            
                            jobList[i+1].quashID = i+1; // sets the quashID as the index + 1
                            jobList[i+1].pid = pid; // sets the pid variable as the pid of the child
                            strcpy(jobList[i+1].command, input); // adds the command to the command variable
                            strcpy(jobList[i+1].formatted, jobbuf); // adds the formatted text of the new job to the formatted variable
                            printf("Background job started: %s\n", jobbuf); // prints that the job started with its information
                            bzero(jobbuf, BSIZE); // empties the buffer
                            break; // ends loop because space was found in jobList
                        }
                    }
                } else if ((waitpid(pid, &status, 0)) == -1) { // parent waits for child to finish executing
                    fprintf(stderr, "Process encountered error...");
                } 
            }
            break;

        //case 2: // Input Redirection - < (Midline Modifier)
            //break;

        case 2: // Output Redirection - > (Midline Modifier)
        // fork inside here to redirect output to something else and then call backExe within that.
        // this will cause a background parent to wait for the background process but the main process with keep going if it's supposed to be ran in the background
            break;
        case 3: // Redirect Output While Appending Output - >> (Midline Modifier)
            break;

    }
    for(int i = 0; i < BSIZE; i++) {
        bzero(parsed[i], BSIZE);
    }
    bzero(leftover, BSIZE);
}

// Comments - # (Midline Modifier)

// Bonus
// Pipes and Redirects Can Be Mixed
// Pipes and Redirects Work with Built In Commands

int main() {
    for (int i = 0; i < MAX_JOBS; i++) { // iterates through all the jobs
        struct job newJob;
        newJob.quashID = 0;
        jobList[i] = newJob; // sets all jobs to quashID of 0 to indicate it's clear
    }
    jobList[MAX_JOBS].quashID = -1;
    jobList[MAX_JOBS].pid = getpid();
    char input[BSIZE]; // creates a character buffer to store input from the user
    printf("PID: %d\n", getpid());
    printf("Welcome...\n");
    while(1) {
        int status; // creates status int for waitpid
        for (int i = 0; i < MAX_JOBS; i++) { // iterates through jobList
            if (jobList[i].quashID > 0) { // checks if the current quashID is not 0, meaning it is storing a process
                if (waitpid(jobList[i].pid, &status, WNOHANG) != 0) { // sends a 0 signal to the PID which does nothing but will return -1 if it fails, meaning the process isn't running
                    printf("Completed: %s\n", jobList[i].formatted); // prints that the process completed
                    jobList[i].quashID = 0; // sets the quashID to 0 to signify the process ended
                    jobList[i].pid = 0; // resets pid value
                    bzero(jobList[i].command, BSIZE); // empties command buffer
                    bzero(jobList[i].formatted, BSIZE); // empties formatted buffer
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