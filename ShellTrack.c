#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>

#define MAX_LINE 80

int main(void){
    // Initializing needed arrays/variables for later
    char *args[MAX_LINE/2 + 1];
    int should_run = 1;
    char *historyArray[10][MAX_LINE];
    int pidArray[10][MAX_LINE];
    int arrayLength = 0;
    *historyArray[0] = "NULL";

    while(should_run){
        // --- STARTUP ---
        memset(args, 0, MAX_LINE/2 +1); // Resetting the args array for every loop (new command)
        char *input;
        size_t size = 0; // Arbitrary size to hold however long of a string
        input = (char *) malloc(size); // gets the size that is needed to make getline happy
        int argsCounter = 0; // Args array length in simple int form
        // Shell print
        printf("ShellTrack>");
        fflush(stdout);

        // --- INPUT ---
        getline(&input, &size, stdin);
        // Creating a history string to easily push the command into the history array
        char *forHistory;
        forHistory = (char *) malloc(size);
        forHistory = strcpy(forHistory,input);
        char* word = strtok(input, " \n"); // Gets first word while delimiting with a space and newline
        // Checking for history entries / exit clause
        // This could be better optimized as a switch especially if we'd like to scale it for further use
        if(NULL == word){ // Checking for just "\n" input
            continue;
        }
        else if(strcmp("exit", word) == 0){ // exit clause
            should_run = 0;
            continue;
        }
        else if(strcmp("history", word) == 0){ // history clause
            if(arrayLength == 0){
                printf("No commands in history!\n");
                continue;
            }
            printf("%-3s %-8s %s\n", "ID", "PID", "Command");
            for (int i = 0; i < arrayLength; ++i) {
                printf("%-3d %-8d %s", i+1, *pidArray[i], *historyArray[i]);
            }
            continue; // Continuing to the top of the loop to prevent from forking
        }
        else if(strcmp("!!", word) == 0){ // !! history
            if(arrayLength == 0){ // Empty history check
                printf("No command in history!\n");
                continue;
            }
            else{
                // Takes Input from the first in history prints input and runs through our normal args input
                input = strcpy(input,*historyArray[0]);
                printf("INPUT: %s", input);
                forHistory = strcpy(forHistory,input);
                word = strtok(input, " \n");
                while (word != NULL) {
                    args[argsCounter] = word;
                    argsCounter++;
                    word = strtok(NULL, " \n");
                }
            }
        }
        else if('!' == word[0]){ // !N history
            if(isdigit(word[1])){ // Checking for if the N is truly a digit
                int number;
                if(strcmp(word, "!10") == 0){// Check as if not we accidentally remove the 0 from the 10
                    number = 10;
                }
                else {
                    number = word[1] - '0'; // Negates the 0 from string to turn into in
                }
                if (arrayLength == 0){ // Checking if any command is in history
                    printf("No command in history!\n");
                    continue;
                }
                if (arrayLength < number){ // Checking if number entered is within the history array
                    printf("Such a command is not in history!\n");
                    continue;
                }
                // Takes Input from the numberneeded-1 in history prints input and runs through our normal args input
                input = strcpy(input,*historyArray[number-1]);
                printf("INPUT: %s", input);
                forHistory = strcpy(forHistory,input);
                word = strtok(input, " \n");
                while (word != NULL) {
                    args[argsCounter] = word;
                    argsCounter++;
                    word = strtok(NULL, " \n");
                }
            }
        }
        else{ // Continues to get all the args outside of single word prompts
            // Looping through the whole input until the word turns to NULL showing we went through all of it
            while (word != NULL) {
                args[argsCounter] = word;
                argsCounter++;
                word = strtok(NULL, " \n");
            }
        }
        // --- COMMAND EXECUTION ---
        // Creates a child process
        pid_t pid = fork();
        switch (pid) {
            case -1: // Child failed to create
                printf("Child failed to create\n");
                continue;
            case 0: // if PID = 0 successfully made child can continue
                // If execvp returns anything negative instantly prints invalid command and exits child process
                if(execvp(args[0], args) < 0){
                    printf("Invalid command!\n");
                    exit(-1);
                    continue;
                }
            default: // Default for parent to wait and then continue the program after child program has exited
                waitpid(pid, NULL, 0);
                char *history0 = (char *) malloc(size);
                history0 = strcpy(history0, *historyArray[0]);
                // --- HISTORY ---
                if(arrayLength >= 1){
                    for (int i = arrayLength; i > 0; i--) {
                        *historyArray[i] = *historyArray[i-1];
                        *pidArray[i] = *pidArray[i-1];
                    }
                    *historyArray[0] = forHistory;
                    *pidArray[0] = pid;
                    // This is for a weird error that was caused when history was 10 long, Fixes history 2 not getting command
                    if(arrayLength == 10){
                        *historyArray[1] = history0;
                    }
                    if(arrayLength < 10){
                        arrayLength++;
                    }
                }
                else{
                    *historyArray[0] = forHistory;
                    *pidArray[0] = pid;
                    arrayLength++;
                }
                continue;
        }
    }
    return 0;
}