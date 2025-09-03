#define _GNU_SOURCE // enable GNU extensions
#include <stdio.h> // include standard input/output library
#include <stdlib.h> // include standard library
#include <string.h> // include string library
#include <unistd.h> // include unix standard library
#include <sys/stat.h> // include system stat library
#include <dirent.h> // include directory entry library
#include <sys/types.h> // include system types library
#include <sys/wait.h> // include wait library
#include <fcntl.h> // include file control library

#define MAX_INPUT 255 // set max input size

char errorMessage[ ] = "An error has occurred\n"; // error message
char *repoPath = NULL;

// function prototypes
void printPrompt(); // function to print prompt
void parseAndRun(char *input); // function to parse and run input
void exitHandler(char **args); // function to handle exit command
void pathHandler(char **args); // function to handle path command
void lsHandler(); // function to handle ls command
int cmpFunc(const void *a, const void *b); // function to compare game names
void getGameDescription(const char *gamePath, char *description); // function to get game description
void runGame(char **args, char *inputRedirect); // function to run game
void redirectInput(char **args, char *filename); // function to redirect input
void errorAndContinue(); // function to handle errors
int isDirectory(const char *path); // function to check if path is a directory


int main(int argc, char *argv[]) {
    if (argc != 2 || !isDirectory(argv[1])) { // check for correct num of arguments
        errorAndContinue(); // handle error
        exit(1); // exit with error
    }
    repoPath = strdup(argv[1]); // store game directory path
    char *input = NULL;
    size_t len = 0; 
    
    while (1) {
        printPrompt();
        if (getline(&input, &len, stdin) == -1) { // read input
            free(input); // free input
            break; // break on error
        }

        char *trimmed = input; // trim input
        while (*trimmed == ' ' || *trimmed == '\t' || *trimmed == '\n') { // trim leading spaces
            trimmed++;
        }
        if (*trimmed == '\0') continue; // skip empty input
        
        parseAndRun(trimmed); // parse and run input ignoring leading spaces
    }
    free(repoPath); // free repo path
    free(input); // free input
    return 0; // return success
}


void printPrompt() {
    printf("shelf-steam> ");
    fflush(stdout);
}

void parseAndRun (char *input) {
    size_t len = strlen(input); // get length of input
    if (len > 0 && input[len - 1] == '\n') { // check for newline
        input[len - 1] = '\0'; // remove newline
    }

    char *args[MAX_INPUT]; // array to hold arguments
    int argCount = 0; // argument count
    char *token = strtok(input, " \t\n"); // tokenize input using whitespaces as delimiters
    while (token != NULL) { // loop through tokens
        args[argCount++] = token; // add token to args
        token = strtok(NULL, " \t\n"); // get next token
    }
    args[argCount] = NULL; // set last arg to NULL
    
    if (argCount == 0) return; // check for empty input
    
    if (strcmp(args[0], "exit") == 0 ||
        strcmp(args[0], "path") == 0 ||
        strcmp(args[0], "ls") == 0) {
        for (int i = 1; i < argCount; i++) {
            if (strcmp(args[i], "<") == 0) { // redirection is not allowed for built-ins
                errorAndContinue();
                return;
            }
        }
        if (strcmp(args[0], "exit") == 0) {
            if (argCount != 1) {
                errorAndContinue();
                return;
            }
            exitHandler(args);
        } else if (strcmp(args[0], "path") == 0) {
            if (argCount != 2) {
                errorAndContinue();
                return;
            }
            pathHandler(args);
        } else if (strcmp(args[0], "ls") == 0) {
            if (argCount != 1) {
                errorAndContinue();
                return;
            }
            lsHandler();
        }
        return;
    }

    int redirectFlag = 0; // flag for redirection
    char *inputFile = NULL; // marker for input file
    int redirectIndex = -1; // index of redirection operator
    for (int i = 0; i < argCount; i++) { // loop through args
        if (strcmp(args[i], "<") == 0) { // check for redirection operator
            if (redirectFlag) { // check if redirection is already set
                errorAndContinue(); // handle error
                return; // return
            }
            redirectFlag = 1; // set flag
            redirectIndex = i; // set index
        }
    }
    if (redirectFlag) { // check if redirection is set
        if (redirectIndex != argCount - 2) { // check if redirection operator is first
            errorAndContinue(); // handle error
            return; // return
        }
        inputFile = args[redirectIndex + 1]; // get input file
        args[redirectIndex] = NULL; // set redirection operator to NULL
    }
    runGame(args, inputFile); // run game
}

void exitHandler(char **args) {
    if (args[1] != NULL) { // check for extra arguments
        errorAndContinue(); // handle error
        return; // return
    }
    exit(0); // exit with success
}

void pathHandler(char **args) {
    if (args[1] == NULL || args[2] != NULL || !isDirectory(args[1])) { // check for correct num of arguments and if path is a directory
        errorAndContinue(); // handle error
        return; // return
    }
    free(repoPath); // free previous repo path
    repoPath = strdup(args[1]); // set new repo path
    
}

void lsHandler() {
    DIR *dir = opendir(repoPath); // open directory
    if (!dir) { // check if directory is valid
        errorAndContinue(); // handle error 
        return;
    }
    // allocate array to hold game names
    int capacity = 10; 
    int count = 0;
    char **gameNames = malloc(capacity * sizeof(char *)); // allocate memory for game names
    if (!gameNames) { // check if memory allocation failed
        closedir(dir); // close directory
        errorAndContinue(); // handle error
        return; // return
    }

    struct dirent *entry; // struct to hold directory entry
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue; 
        char gamePath[512]; // array to hold game path
        snprintf(gamePath, sizeof(gamePath), "%s/%s", repoPath, entry->d_name); // create game path
        
        struct stat pathStat; // struct to hold path status
        if (stat(gamePath, &pathStat) != 0) continue; // continue to next entry
        if (S_ISDIR(pathStat.st_mode)) continue; // check if entry is a directory

        char *nameCopy = strdup(entry->d_name); // duplicate game name
        if (!nameCopy) continue; // check if memory allocation failed
        if (count >= capacity) { // check if capacity is reached
            capacity *= 2; // double capacity
            char **temp = realloc(gameNames, capacity * sizeof(char *)); // reallocate memory for game names
            if (!temp) {
                free(nameCopy); // free name copy
                continue; // continue to next entry
            }
            gameNames = temp; // set new game names
        }
        gameNames[count++] = nameCopy; // add game name to array
    }
    closedir(dir); // close directory

    qsort(gameNames, count, sizeof(char *), cmpFunc); // sort game names

    for (int i = 0; i < count; i++) { // loop through game names
        char gamePath[512]; // array to hold game path
        snprintf(gamePath, sizeof(gamePath), "%s/%s", repoPath, gameNames[i]); // create game path
        char description[MAX_INPUT]; // variable to hold description
        getGameDescription(gamePath, description); // get game description
        printf("%s: %s\n", gameNames[i], description); // print game name and description
        fflush(stdout); // flush output
        free(gameNames[i]); // free game name
    }
}

int cmpFunc(const void *a, const void *b) { // sort the game names
    const char *nameA = *(const char **)a;
    const char *nameB = *(const char **)b;
    return strcmp(nameA, nameB);
}

void getGameDescription(const char *gamePath, char *description) {
    int pipefd[2]; // array to hold pipe file descriptors
    if (pipe(pipefd) == -1) { // create pipe
        strcpy(description, "(empty)"); // handle error
        return;
    }

    pid_t pid = fork(); // fork process
    if (pid < 0) { // check if fork failed
        strcpy(description, "(empty)"); // handle error
        return;
    }

    if (pid == 0) { // check if child process
        close(pipefd[0]); // close read end of pipe
        dup2(pipefd[1], STDOUT_FILENO); // redirect stdout to pipe
        dup2(pipefd[1], STDERR_FILENO); // close write end of pipe
        close(pipefd[1]); // close write end of pipe

        execl(gamePath, strrchr(gamePath, '/') + 1, "--help", NULL); // execute file command
        exit(1); // exit with error
    }

    close(pipefd[1]); // close write end of pipe
    wait(NULL); // wait for child process

    ssize_t bytesRead = read(pipefd[0], description, MAX_INPUT - 1); // read from pipe
    close(pipefd[0]); // close read end of pipe

    if (bytesRead > 0) {
        description[bytesRead] = '\0';
        description[strcspn(description, "\n")] = '\0'; 

        char *gameName = strrchr(gamePath, '/') + 1; // get game name

        char tempBuffer[MAX_INPUT]; // buffer to hold temp description
        strncpy(tempBuffer, description, sizeof(tempBuffer) - 1); // copy description to temp buffer
        tempBuffer[MAX_INPUT - 1] = '\0'; // null terminate temp buffer

        char *firstWord = strtok(tempBuffer, " "); // get first word
        char *remainingDesc = strtok(NULL, ""); // get remaining description

        if (firstWord && remainingDesc) {
            if (strcmp(firstWord, gameName) == 0) {
                snprintf(description, MAX_INPUT, "%s %s", firstWord, remainingDesc);
            } else {
                snprintf(description, MAX_INPUT, "%s %.*s", gameName, MAX_INPUT - (int)strlen(gameName) - 2, tempBuffer);
            }
        }
    } else {
        strcpy(description, "(empty)");
    }
}


void runGame(char **args, char *inputRedirect) {
    if (!repoPath || !args[0]) { // check if repo path is NULL or game name is NULL
        errorAndContinue(); // handle error
        return;
    }

    char gamePath[512]; // array to hold game path
    snprintf(gamePath, sizeof(gamePath), "%s/%s", repoPath, args[0]); // create game path

    if (access(gamePath, X_OK) != 0) { // check if game is executable
        errorAndContinue(); // handle error
        return;
    }

    pid_t pid = fork(); // fork process
    if (pid < 0) { // check if fork failed
        errorAndContinue(); // print error message
        return; 
    }
    if (pid == 0) { // check if child process
        if (inputRedirect) {
            int fd = open(inputRedirect, O_RDONLY); // open file for reading
            if (fd == -1) { // check if file descriptor is invalid
                errorAndContinue(); // print error message
                exit(1); // exit with error
            }
            dup2(fd, STDIN_FILENO); // duplicate file descriptor to redirect input
            close(fd); // close file descriptor
        }
        execvp(gamePath, args); // execute game
        errorAndContinue(); // print error message
        exit(1); // exit with error
    } else { // parent process
        wait(NULL); // wait for child process
    }
}

void redirectInput(char **args, char *filename) {
    int fd = open(filename, O_RDONLY); // open file for reading
    if (fd == -1) { // check if file descriptor is invalid
        errorAndContinue(); // print error message
        return;
    }

    pid_t pid = fork(); // fork process
    if (pid < 0) { // check if fork failed
        errorAndContinue(); // print error message
        close(fd); // close file descriptor
        return; 
    }

    if (pid == 0) { // check if child process
        dup2(fd, STDIN_FILENO); // duplicate file descriptor to redirect input
        close(fd); // close file descriptor
        execvp(args[0], args); // execute game
        errorAndContinue(); // print error message
        exit(1); // exit with error
    } else { // parent process
        close(fd); // close file descriptor
        wait(NULL); // wait for child process
    }
}

void errorAndContinue() {
    write(STDERR_FILENO, errorMessage, strlen(errorMessage)); // print error message
    fflush(stderr); // flush error output
}

int isDirectory(const char *path) { 
    struct stat pathStat; // struct to hold path status
    return (stat(path, &pathStat) == 0 && S_ISDIR(pathStat.st_mode)); // check if path is a directory
}