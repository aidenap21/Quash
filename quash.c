#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <sys/wait.h>

#define BSIZE 256
#define MAX_JOBS 10

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
                printf("Invalid directory..."); // state that the directory is invalid
                return 2; // returns 2 to signify error in input parameters
            }
            return 0;

        case 6: // Print Path of Current Directory - pwd (Simple command)
            char directorybuf[BSIZE]; // creates a buffer for the current working directory
            bzero(directorybuf, BSIZE); // empties the buffer
            getcwd(directorybuf, BSIZE); // gets the current working directory and stores it in the buffer
            

        case 7: // List Directory Contents - ls
            ls(input[1]); // calls ls with the first index value which is the letter parameter

        case 8:// Send POSIX Signal to Process - kill (Simple command)
            kill(input[1], input[2]); // VERIFY SIGNIAL IS SENT SOMEHOW??
            return 0;
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

int parser(char *input, char parsed[256][256]) { //parameters for input string and matrix to output the parsed data to

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
        strcpy(parsed[i], temp); //Copy the current word into the parsed array
        
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
                    return 1;
                    
                case 1: // < case
                    return 2;
                
                case 2: // > case
                    return 3;
                    
                case 3: // >> case
                    return 4;
            }
        }
        i++;
    }
    return 0;

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