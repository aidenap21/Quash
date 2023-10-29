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
void forExe(char exe[][BSIZE], int numberOfItems) { // takes in executable name and arguments as a string. Also takes in output that will be printed or passed somewhere else
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
            outputFile = freopen(exe[i+1], "w", stdout); // exe[i+1] is the name of the output file and opens in write mode
            if (outputFile == NULL) {
                printf("Error opening file...\n");
                exit(1);
            }
        } else if (nextIsOutFile == 2) { // runs if >> was found
            FILE *outputFile;
            outputFile = freopen(exe[i+1], "a", stdout); // exe[i+1] is the name of the output file and opens in append mode
            if (outputFile == NULL) {
                printf("Error opening file...\n");
                exit(1);
            }
        }
        //printf("CHILD PID IN FOREXE: %d\n", getpid());
        if (execvp(exePtr[0], exePtr) < 0) { // calls execvp on passed in executable with unparsed as parameters but catches error if exec fails
            printf("Error executing...\n"); // prints statement that exec fails
            exit(1); // exits child process since it failed
        }

    } else if (p < 0) { // parent process
        printf("Fork failed...\n");
    }
    
    if ((waitpid(p, &status, 0)) == -1) { // parent waits for child to finish executing
            fprintf(stderr, "Process encountered error...\n");
    }
    
    return;
}

// Background Executables - & (In Progress)  need to figure out how to redirect output to the output buffer and properly add to job list
void backExe(char exe[][BSIZE], char* unparsed, int numberOfItems) { // takes in executable name and arguments as a string. Also takes in unparsed to be passed into job list and takes in output that will be printed or passed somewhere else
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
            if (outputFile == NULL) {
                printf("Error opening file...\n");
                exit(1);
            }
        } else if (nextIsOutFile == 2) {
            FILE *outputFile;
            outputFile = freopen(exe[i+1], "a", stdout);
            if (outputFile == NULL) {
                printf("Error opening file...\n");
                exit(1);
            }
        }

        if (execvp(exePtr[0], exePtr) < 0) { // calls execvp on passed in executable with parameters but catches error if exec fails
            printf("Error executing background process...\n"); // prints statement that exec fails
            exit(1); // exits child process since it failed 
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

    return;
}

void handlePipes(char exe[][BSIZE], int numberOfItems, int numPipes) { // if parsing everything then check last index to see if it has & to run background
    char currentItem[BSIZE]; // will store word of current iteration
    int nextStart = 0, curPipe = 0, i = 0, curIndex = 0, nextIsVar = 0, nextIsOutFile = 0, status, pipeArray[numPipes][2]; // holds current index of exePtr, flag to check if next item is $, status of child
    pid_t pidArray[numPipes + 1]; // array of pipes, read from previous index and then write to the next one to pass between the loop

    for (int j = 0; j < numPipes; j++) {
        if (pipe(pipeArray[j]) == -1) { // initializes the pipes
            printf("Pipe failed...\n");
        }
    }

    for (int j = 0; j < numPipes + 1; j++) {
        char *exePtr[BSIZE];  // array of pointers to strings

        for (int k = 0; k < BSIZE; k++) {
            exePtr[k] = NULL;
        }

        curIndex = 0;

        for (i; i < numberOfItems; i++) {
            bzero(currentItem, BSIZE);

            if (exe[i][0] == '|') { // checks if pipe
                i++; // increases i for next loop starting location
                break; // breaks

            } else if (exe[i][0] == '$') { // checks if environmental variable
                nextIsVar = 1; // marks flag


            } else if (exe[i][0] == '<') { // runs if input symbol
                continue; // continues to not add it to the array

            } else if (strcmp(exe[i], ">>") == 0) { // checks if append output cae is next
                nextIsOutFile = 2; // marks flag for next iteration
                break;

            } else if (exe[i][0] == '>') { // checks if output case is next
                nextIsOutFile = 1; // marks flag for next iteration
                break;
            
            } else if (exe[i][0] == '&') {
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
            
            pidArray[j] = fork(); // calls fork on pid p

            if (pidArray[j] == 0) { // child process
                char pipeTest[BSIZE];
                bzero(pipeTest, BSIZE);

                if (j == 0) { // first process only writes to pipe
                    for (int k = 1; k < numPipes; k++) { // closes all but the first pipe
                        close(pipeArray[k][0]);
                        close(pipeArray[k][1]);
                    }
                    close(pipeArray[0][0]); // closes read end of pipe

                    dup2(pipeArray[0][1], 1); // sets write end to std out

                    close(pipeArray[0][1]); // closes write end

                } else if (j == numPipes) { // last process only reads from pipe
                    for (int k = 0; k < numPipes - 1; k++) { // closes all but the last pipe
                        close(pipeArray[k][0]);
                        close(pipeArray[k][1]);
                    }
                    close(pipeArray[numPipes - 1][1]); // closes write end of pipe

                    dup2(pipeArray[numPipes - 1][0], 0); // sets read end to std in

                    close(pipeArray[numPipes - 1][0]); // closes read end
                }
                
                else {
                    for (int k = 0; k < numPipes; k++) { // closes all but current and previous pipes
                        if (k != j && k != j - 1) {
                            close(pipeArray[k][0]);
                            close(pipeArray[k][1]);
                        }
                    }
                    close(pipeArray[j - 1][1]); // closes previous pipe write
                    close(pipeArray[j][0]); // closes current pipe read

                    dup2(pipeArray[j - 1][0], 0); // reads from previous pipe
                    dup2(pipeArray[j][1], 1); // sets write to std out

                    close(pipeArray[j - 1][0]);
                    close(pipeArray[j][1]);
                }

                if (execvp(exePtr[0], exePtr) < 0) { // calls execvp on passed in executable with unparsed as parameters but catches error if exec fails
                    printf("Error executing...\n"); // prints statement that exec fails
                    exit(1); // exits child process since it failed
                }

        } else if (pidArray[j] < 0) { // parent process
            printf("Fork failed...\n");
        }
        
        if ((waitpid(pidArray[j], &status, WNOHANG)) == -1) { // doesn't wait for child to end just checks if there was an error
                fprintf(stderr, "Process encountered error...\n");
            }
            
    }

    for (int j = 0; j < numPipes; j++) { // closes all pipes in parent
        close(pipeArray[j][0]);
        close(pipeArray[j][1]);
    }

    if ((waitpid(pidArray[numPipes], &status, 0)) == -1) { // only waits the last process since the rest of the processes will have to wait based on pipe read anyway
            fprintf(stderr, "Process encountered error...\n");
    }
}

// Print String - echo (In Progress)
int echoString(char parsed[][BSIZE], int numberOfItems) { // takes in string to print (needs to remove "echo" work from start of string)
    char output[BSIZE], c; // will store word of current iteration
    int nextIsVar = 0, nextIsInFile = 0, nextIsOutFile = 0, outputRedirect; // flag to check if value following $ is environmental variable
    bzero(output, BSIZE); // empties the buffer
    FILE* outputFile;
    
    for (outputRedirect = 0; outputRedirect < numberOfItems; outputRedirect++) { // iterates through parsed to find output redirect
        if (strcmp(parsed[outputRedirect], ">>") == 0) { // checks if append output cae is next
            nextIsOutFile = 2; // marks flag for next iteration
            break;

        } else if (parsed[outputRedirect][0] == '>') { // checks if output case is next
            nextIsOutFile = 1; // marks flag for next iteration
            break;
        }
    }

    if (nextIsOutFile == 1) { // runs if > was found
        outputFile = fopen(parsed[outputRedirect + 1], "w"); // opens file in write mode
        if (outputFile == NULL) {
            printf("Error opening output file...\n");
            return 1;
        }

    } else if (nextIsOutFile == 2) { // runs if >> was found
        outputFile = fopen(parsed[outputRedirect + 1], "a"); // opens file in write mode
        if (outputFile == NULL) {
            printf("Error opening output file...\n");
            return 1;
        }
    }
            
    for (int i = 1; i < numberOfItems; i++) { // iterates through parsed starting at index 1 to not print out the echo command word
        if (parsed[i][0] == '$') { // checks if environmental variable
            nextIsVar = 1; // marks flag for next iteration

        } else if (strcmp(parsed[i], ">>") == 0) { // checks if append output cae is next
            break;

        } else if (parsed[i][0] == '>') { // checks if output case is next
            break;
            
        } else if (parsed[i][0] == '<') { // checks if input case is next
            nextIsInFile = 1; // marks flag for next iteration

        } else if (nextIsVar == 1) { // runs if item is variable
            if (nextIsOutFile > 0) {
                fprintf(outputFile, "%s", getenv(parsed[i])); // gets environmental variable value and adds to output
            } else {
                printf("%s", getenv(parsed[i])); // gets environmental variable value and adds to output
            }
            nextIsVar = 0; // resets flag

        } else if (nextIsInFile == 1) { // runs if item is file
            FILE *inputFile; // creates file type
            inputFile = fopen(parsed[i], "r"); // opens file in read mode
            if (inputFile == NULL) {
                printf("Error opening input file...\n");
                return 1;
            }

            if (nextIsOutFile > 0) {
                c = fgetc(inputFile); 
                while (c != EOF) 
                { 
                    fputc(c, outputFile); 
                    c = fgetc(inputFile); 
                } 
            } else {
                fgets(output, BSIZE, inputFile); // reads from the file and stores it in the buffer
                printf("%s", output);   
            }

            fclose(inputFile); // closes the file
            break; 

        } else { // runs if next item is just text
            if (nextIsOutFile > 0) {
                fprintf(outputFile, "%s ", parsed[i]); // gets environmental variable value and adds to output
            } else {
                printf("%s ", parsed[i]); // gets environmental variable value and adds to output
            }
        }
    }

    if (nextIsOutFile == 0) { // runs if the it didn't output to a file
        printf("\n"); // prints the output
    }
    if (nextIsOutFile > 0) {
        fprintf(outputFile, "\n"); // prints the output
        fclose(outputFile);
    }
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
            int status; // creates status to check if signaled process isn't running anymore
            if (kill(atoi(parsed[2]), atoi(parsed[1])) == -1) { // calls kill with PID and then signal from input after they've been converted to ints
                return 2; // returns 2 to signify error in calling kill
            } 
            usleep(1000); // sleeps for some miliseconds to make sure it updates if the process ended
            if (waitpid(atoi(parsed[2]), &status, WNOHANG) != 0) { // checks if the process was ended 
                for (int i = 0; i < MAX_JOBS; i++) {
                    if (jobList[i].pid == atoi(parsed[2])) { // finds pid of ended process removes from jobList without printing completed
                        jobList[i].quashID = 0; // sets the quashID to 0 to signify the process ended
                        jobList[i].pid = 0; // resets pid value
                        bzero(jobList[i].command, BSIZE); // empties command buffer
                        bzero(jobList[i].formatted, BSIZE); // empties formatted buffer
                    }
                }
            }
            return 0; // returns 0 to signify success
    }
}

int parser(char *input, char parsed[BSIZE][BSIZE], int *numPipes, int *pLen)  //parameters for input string and matrix to output the parsed data to
{
    int inLen = strlen(input), curStart = 0, curWord = 0; 
    int pipeCount = 0;
    
    if(inLen == 0) //check if the input is empty
    { return 5; }
    
    int single = 0, dub = 0, count = 0, environs = 0, postQuote = 0, postSpace = 0; //single to check for '', dub to check for "", count to see if there is already a " or ' in the command
    
    for (int j = 0; j < inLen; j++) //Loop thru entire input
    { 
        if(input[j] == ' ' && dub == 0 && single == 0) { // checks if current character is space and isn't in quotes
            if (postSpace == 1) { // removes extra spaces between terms
                curStart = j + 1; // resets the start value for the next word
                continue; // continues that way flags aren't reset yet
            } else if (postQuote == 1) { // removes extra space between terms
                curStart = j + 1; // resets the start value for the next word
                continue; // continues that way flags aren't reset yet
            }
            strncpy(parsed[curWord], input + curStart, j - curStart); // truncates the current word and stores it
            curStart = j + 1; // resets the start value for the next word
            curWord++; // increments the next available word position
            postSpace = 1;
            continue; // continues that way flags aren't reset

        } else if(input[j] == '|' && dub == 0 && single == 0) { // runs if on | and not in quotes
            //printf("found | symbol\n");
            pipeCount++; // increments the number of pipes
            //printf("pipeCount: %d\n", pipeCount);

        } else if(input[j] == '$' && dub == 0 && single == 0) {
            parsed[curWord][0] = '$'; // sets value to $ separate from following variable name itself
            curWord++; // increments curWord to use the next position
            curStart = j + 1; // resets curStart
            environs = 1; // sets environs to 1 as flag to stop when / is found
        
        } else if(input[j] == '/' && dub == 0 && single == 0 && environs == 1) {
            strncpy(parsed[curWord], input + curStart, j - curStart); // truncates the current word and stores it
            curStart = j; // resets the start value for the next word but uses / as start instead of next character
            curWord++; // increments the next available word position
            environs = 0;
        
        } if(input[j] == '\'' && dub == 0 && environs == 0) { //Case for ''
            if (count == 0) // beginning quote
            { 
                single = 1; 
                count++;
                if (postSpace == 0 && postQuote == 0) { // makes sure previous wasn't space or quote so blank space isn't added
                    strncpy(parsed[curWord], input + curStart, j - curStart); // truncates the current word and stores it
                    curStart = j + 1; // resets the start value for the next word
                    curWord++; // increments the next available word position
                }
            }
            else // ending quote
            { 
                strncpy(parsed[curWord], input + curStart + 1, j - curStart - 1); // adds extra 1 to start to remove quote characters
                curStart = j; // resets the start value for the next word
                curWord++; // increments the next available word position
                single = 0;
                postQuote = 1; // marks flag if quote was last added to avoid white space adding
                count--;
                continue; // continues that way flags aren't reset
            }
        
        } else if(input[j] == '\"' && single == 0 && environs == 0) { //Case for ""
            if (count == 0) {// beginning quote 
                dub = 1; 
                count++;
                if (postSpace == 0 && postQuote == 0) { // makes sure previous wasn't space or quote so blank space isn't added
                    strncpy(parsed[curWord], input + curStart, j - curStart); // truncates the current word and stores it
                    curStart = j; // resets the start value for the next word
                    curWord++; // increments the next available word position
                }
            } else { // ending quote 
                strncpy(parsed[curWord], input + curStart + 1, j - curStart - 1); // adds extra 1 to start to remove quote characters
                curStart = j + 1; // resets the start value for the next word            
                curWord++; // increments the next available word position
                dub = 0; 
                postQuote = 1; // marks flag if quote was last added to avoid white space adding
                count--;
                continue; // continues that way flags aren't reset
            } 

        } else if(single == 0 && dub == 0 && environs == 0) {//Case for when the input is not surrounded by quotes
            if (input[j] == '#') //Check for comments
            {
                if(j == 0) //Return if there is a comment at the very start
                { 
                    *pLen = curWord;
                    return 1;
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

        if (postQuote == 1) {
            postQuote = 0;
        }
        if (postSpace == 1) {
            postSpace = 0;
        }
    }
    if (postQuote == 0 && postSpace == 0) { // runs if the last character wasn't a closing quote to avoid blank space token
        strncpy(parsed[curWord], input + curStart, inLen - curStart); // truncates the current word and stores it
        curWord++;
    }
    *pLen = curWord; // sets number of items as current word number
    *numPipes = pipeCount;
    return 0; // returns 0 to signify no midline modifiers
}

void parseThenPass(char* input) { // parses input and runs corresponding command/executable
    char parsed[BSIZE][BSIZE]; // creates an array that will store the tokenized input from parser function
    int numberOfItems = 0, numPipes = 0, background = 1, nextIsOutFile = 0, outputRedirect; // creates variable to store number of parsed items and if there is an & at the end for midline modifiers
    if (parser(input, parsed, &numPipes, &numberOfItems) == 1) { // calls parser and stores the return value to check if pipes or redirection exist in the input
        return; // returns because the output of the parser was empty
    }
    FILE *outputFile; // creates outputFile

    if (numPipes == 0) { // runs if no pipes

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
            for(int i = 0; i < BSIZE; i++) {
                bzero(parsed[i], BSIZE);
            }
            return;

    } else if (numPipes > 0) { // Pipes - | (Midline Modifier)
        int status;
        pid_t pid = fork();
        if (pid == 0) {
            int outputRedirect;
            FILE* outputFile;
            
            for (outputRedirect = 0; outputRedirect < numberOfItems; outputRedirect++) { // iterates through parsed to find output redirect
                if (strcmp(parsed[outputRedirect], ">>") == 0) { // checks if append output cae is next
                    nextIsOutFile = 2; // marks flag for next iteration
                    break;

                } else if (parsed[outputRedirect][0] == '>') { // checks if output case is next
                    nextIsOutFile = 1; // marks flag for next iteration
                    break;
                }
            }

            if (nextIsOutFile == 1) { // runs if > was found
                outputFile = freopen(parsed[outputRedirect+1], "w", stdout); // opens file name from parsed[outputRedirect+1] in write mode
                if (outputFile == NULL) { // runs if error opening file
                    printf("Error opening output file...\n");
                    for(int i = 0; i < BSIZE; i++) {
                        bzero(parsed[i], BSIZE);
                    }
                    return;
                }
            } else if (nextIsOutFile == 2) { // runs if >> was found
                outputFile = freopen(parsed[outputRedirect+1], "a", stdout); // opens file name from parsed[outputRedirect+1] in append mode
                if (outputFile == NULL) { // runs if error opening file
                    printf("Error opening output file...\n");
                    for(int i = 0; i < BSIZE; i++) {
                        bzero(parsed[i], BSIZE);
                    }
                    return;
                }
            }

            handlePipes(parsed, numberOfItems, numPipes);
            exit(0);

        } else if (pid > 0) {
            if (parsed[numberOfItems - 1][0] == '&') {
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
        
    }

    for(int i = 0; i < BSIZE; i++) {
        bzero(parsed[i], BSIZE);
    }
    return;
}

int main() {
    for (int i = 0; i < MAX_JOBS; i++) { // iterates through all the jobs
        struct job newJob;
        newJob.quashID = 0;
        jobList[i] = newJob; // sets all jobs to quashID of 0 to indicate it's clear
    }
    char input[BSIZE]; // creates a character buffer to store input from the user
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
    }

    return 0;
}