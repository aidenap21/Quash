#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>


// List Directory Contents - ls
int ls(char arg) { // takes in single letter argument

}

// Foreground Executables
int forexe(char *exe, char *arg) { // takes in executable name and arguments as a string
    pid_t p = fork();
    if(p == 0) {

    }
}

// Background Executables - &
int backexe(char *exe, char *arg) { // takes in executable name and arguments as a string

}


// Print String - echo
int echoString(char *string) { // takes in string to print

}

// Set Value of Environmental Variable - export
int export(char *var, char *val) { // takes in variable name and value to update with (not positive about variable type)

}

// Change Working Directory - cd
int changeDir(char *path) { // takes in new path to navigate to

}

// Print Path of Current Directory - pwd
int workingDir() {

}

// Print All Running Background Process - jobs
int printJobs() {

}

// Send POSIX Signal to Process - kill
int sendSig(int signal, int pid) { // takes in POSIX signal and PID to send signal to

}

int parser(char *input) { // takes in string to parse

}
// Input Redirection - < (Build into parser)
// Output Redirection - > (Build into parser)
// Redirect Output While Appending Output - >> (Build into parser)
// Pipes - | (Build into parser)
// Comments - # (Build into parser)
// Terminate Quash - quit, exit (Build into parser)


// Bonus
// Pipes and Redirects Can Be Mixed
// Pipes and Redirects Work with Built//In Commands

int main() {

}