#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <search.h>
#include <fcntl.h>

#define maxLength 256
#define green "\x1B[32m"
#define blue "\x1B[34m"
#define white "\x1B[0m"
#define bold "\x1B[1m"

char *currentPath;
char *user;

//["ls" , "-a" , "-l"] is three arguments command
char *args[50];
int argsNO = 0;
char *hashTable[maxLength];
int background = 0 , fileExist = 0;

int hashFunction(char *input);

void printColored(char *text, char *color);

void freeMemory();

void freeArgs() ;

void setDirectory();

void setupEnvironment();

char *readInput();

_Noreturn void shell();

void parseInput(char *input);

void executeCommand();

void executeShellBuiltin();

char *parseVars(char *input);

void parseArguments(int first);

void procExit();

void childLog();

int main() {
    //set children handlers when signaling
    signal(SIGCHLD, procExit);

    //opening file for logging
    fileExist = open("log.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    currentPath = malloc(sizeof(char) * maxLength);
    user = malloc(sizeof(char) * maxLength);

    setupEnvironment();
    shell();

    return 0;
}

//the main shell loop
 void shell() {
    do {

        //printed colored path and user
        printColored("", bold);
        printColored(user, green);
        printColored(currentPath, blue);
        printColored(" ", white);

        parseInput(readInput());

        //knowing if the command is foreground or background by -&- argument
        background = !(strcmp(args[argsNO - 1], "&"));
        if (background) {
            args[argsNO - 1] = NULL;
            argsNO -- ;
        }

        //parse first argument only to if its value equal executable command
        parseArguments(1);

        if (!strcmp(args[0], "exit")) {
            freeMemory();
            kill(getppid(), SIGKILL);
        } else if (!strcmp(args[0], "cd") || !strcmp(args[0], "echo") || !strcmp(args[0], "export")) {
            executeShellBuiltin();
        } else {
            //parsing all command arguments
            parseArguments(0);
            executeCommand();
        }
    } while (1);
}

//hash function of the hash table for export and echo commands
int hashFunction(char *input) {
    long long sum = 0;
    for (int i = 0; input[i] != '\0'; ++i) {
        long long x = (i + 1) * (i + 1);
        sum += input[i] * x;
    }
    return (int) (sum % maxLength);
}

//print colored text
//the color it takes defined in the start of the program
void printColored(char *text, char *color) {
    printf("%s", color);
    printf("%s", text);
}

void freeMemory() {
    free(user);
    free(currentPath);
    freeArgs() ;
}

void freeArgs(){
    for (int i = 0; i < 50; ++i) {
        free(args[i]);
    }
}

//set-up path , username and host before starting
void setupEnvironment() {
    int userPointer = 0;
    char buffer[maxLength];

    //getting username
    struct passwd *p = getpwuid(getuid());  // Check for NULL!
    char *userName = p->pw_name;
    for (; userName[userPointer] != '\0'; ++userPointer)
        user[userPointer] = userName[userPointer];
    user[userPointer++] = '@';

    //getting computer name
    gethostname(buffer, sizeof(buffer));  // Check the return value!
    for (int i = 0; i < strlen(buffer); ++i) {
        user[userPointer++] = buffer[i];
    }
    user[userPointer] = ':';

    //getting current directory path
    setDirectory();
}

//getting current directory using -getcwd-
void setDirectory() {
    char buffer[maxLength];
    getcwd(buffer, sizeof(buffer));
    strcpy(currentPath, buffer);
    currentPath[strlen(buffer)] = '$';
    currentPath[strlen(buffer) + 1] = '\0';  //terminating string end
}


//read input by using -getline-
char *readInput() {
    char *p;
    size_t size = maxLength;
    p = malloc(sizeof(char) * size);
    int charsRead = (int) getline(&p, &size, stdin);
    p[charsRead - 1] = '\0';    //converting '\n' to null
    return p;
}

//parse input into separating arguments
//e.g.
// ["ls -a -h"] ----> ["ls" , "-a" , "-h"]
//["export x=/"hello $y/""] ----> ["export" , "x=/"hello $y/""]
void parseInput(char *input) {
    int j = 0;
    argsNO = 0;                     //to record number of command arguments
    char *arg;                      //buffer for single argument
    arg = malloc(sizeof(char) * 50);

    memset(arg, 50, sizeof(char));
    for (int in = 0; in < strlen(input); ++in) {

        //if there is -/"- take all input until second /"
        if (input[in] == '"') {
            arg[j++] = input[in++];
            while (input[in] != '"') {
                arg[j++] = input[in++];
            }
            arg[j++] = input[in++];

            //if there is space save last argument and start another one
        } else if (input[in] == ' ') {
            arg[j] = '\0';          //terminating string end
            args[argsNO++] = strdup(arg);
            j = 0;
        } else {
            arg[j++] = input[in];
        }
    }
    arg[j] = '\0';                  //terminating string end
    args[argsNO++] = strdup(arg);
    free(arg);
    free(input) ;

    //terminating the array for execvp since the argument after the last must be NULL
    args[argsNO] = NULL;
}

//execute non-built-in commands using execvp
void executeCommand() {
    int childId = fork(), status;

    //if child
    if (childId == 0) {
        execvp(args[0], args);

        //if the child reached here then there is no such command
        printf("%s : command not found \n", args[0]);
        exit(0);
    } else {
        //if background command don't wait
        if (!background)waitpid(childId, &status, 0);
        background = 0;
    }
}

//execute built-in commands
// cd - echo - export
void executeShellBuiltin() {

    if (!(strcmp(args[0], "cd"))) {
        //parse second argument to check -$- cases
        args[1] = parseVars(args[1]);

        //if -cd- or -cd ~- go home directory of the user
        if (!(strcmp(args[1], "")) || !(strcmp(args[1], "~"))) {
            char *home = malloc(sizeof(char) * 50);
            home = strdup("/home/");
            struct passwd *p = getpwuid(getuid());  // Check for NULL!
            char *userName = p->pw_name;

            args[1] = strdup(strcat(home, userName));
            free(home);
        }

        int status = chdir(args[1]);

        //if 0 then there is no errors
        if (status == -1)
            printf("%s: no such file or directory\n", args[1]);
        else
            //set new directory into user variable
            setDirectory();

        free(args[1]) ;

    } else if (!(strcmp(args[0], "echo"))) {

        //parse all arguments for -$- cases
        for (int i = 1; args[i] != NULL; ++i) {
            char *temp = strdup(args[i]);

            //to handle double quotes cases
            //e.g. echo "x5" ---> echo x5
            if (temp[0] == '"') {
                int j = 1;
                for (; temp[j] != '"' && temp[j] != '\0'; ++j) {
                    temp[j - 1] = temp[j];
                }
                temp[j - 1] = '\0';
            }

            temp = parseVars(temp);
            printf("%s ", temp);
            free(temp) ;
        }
        printf("\n");

        //-export- case
    } else {
        char *equalSign;
        for (int i = 1; args[i] != NULL; ++i) {
            //checking for equal sign and save the substring to end
            equalSign = memchr(args[i], '=', strlen(args[i]));

            //if there is no equal skip argument
            if (equalSign == NULL) continue;

            else {
                char *key;
                //take chars until before -=-
                key = strtok(strdup(args[i]), "=");
                key = parseVars(key);

                if (!(strcmp(key, ""))) {
                    printf("export : \'%s\' : not a valid identifier \n", equalSign);
                    free(key);
                    return;

                } else {
                    //handle case of double quotes
                    int start = 1;
                    if (equalSign[1] == '"') start = 2;

                    int j = start;
                    for (; equalSign[j] != '"' && equalSign[j] != '\0'; ++j) {
                        equalSign[j - start] = equalSign[j];
                    }
                    equalSign[j - start] = '\0';        //terminating string end

                    //parse -$- vars
                    equalSign = parseVars(equalSign);
                }

                //store the data into our hash table
                hashTable[hashFunction(key)] = strdup(equalSign);
                free(key) ;
                free(equalSign) ;
                equalSign = NULL ;
            }
        }
    }
}

//parsing input with '$' vars
//e.g. if x in hash table equal c5
// $x="$y v" ---> c5="v"        -we don't have y-
char *parseVars(char *input) {
    if (input == NULL) return "";

    char *result = malloc(sizeof(char) * 50);
    result[0] = '\0' ;
    char *buffer = malloc(sizeof(char) * 50);
    int tempPointer = 0;

    for (int i = 0; input[i] != '\0'; ++i) {
        if (input[i] != '$' && input[i] != ' ') {
            buffer[tempPointer++] = input[i];

        } else if (input[i] == '$') {
            i++;
            //store previous buffer
            buffer[tempPointer] = '\0';
            result = strcat(result, strdup(buffer));
            tempPointer = 0;

            //take the full variable key
            while (input[i] != '$' && input[i] != '\0' && input[i] != ' ') {
                buffer[tempPointer++] = input[i++];
            }
            i--;
            buffer[tempPointer] = '\0';                       //terminating string end

            //search in hash table for the variable
            int index = hashFunction(strdup(buffer));
            if (hashTable[index] != NULL) result = strcat(result, strdup(hashTable[index]));
            tempPointer = 0;

        } else if (input[i] == ' ') {
            //if there is nothing in the buffer
            if (tempPointer == 0) continue;

            buffer[tempPointer++] = ' ';
            buffer[tempPointer] = '\0';                     //terminating string end
            result = strcat(result, strdup(buffer));
            tempPointer = 0;
        }
    }

    buffer[tempPointer] = '\0';                             //terminating string end
    //check if the buffer isn't empty
    if (tempPointer != 0)result = strcat(result, strdup(buffer));
    free(buffer);
    return result;
}

//parse argument of commands
//e.g. if (x = ls -h)
//["$x" ,"-a"] -----> ["ls" , "-a" , "-h"]
//first : if we want to parse first argument only or all arguments
void parseArguments(int first) {
    for (int i = 0; (args[i] != NULL && !first) || (i < 1 && first); ++i) {
        args[i] = parseVars(args[i]);

        char *buffer, *temp;
        buffer = malloc(sizeof(char) * maxLength);
        temp = strdup(args[i]);
        int pointer = 0, num = 0;
        for (int j = 0; temp[j] != '\0'; ++j) {
            if (temp[j] != ' ') {
                buffer[pointer++] = temp[j];
            } else {
                buffer[pointer] = '\0';                 //terminating string end

                //if first argument to parse put it in its place like the example above the function
                if (num == 0)
                    args[i] = strdup(buffer);
                else
                    args[argsNO++] = strdup(buffer);
                pointer = 0, num++;
            }
        }
        buffer[pointer] = '\0';                         //terminating string end
        if (num == 0)
            args[i] = strdup(buffer);
        else
            args[argsNO++] = strdup(buffer);
        args[argsNO] = NULL;
        pointer = 0;
        free(buffer);
    }
}

//to handle zombie children
void procExit() {
    int wstat;
    pid_t pid;

    while (1) {
        //use wait3 here is equal to waitpid
        //Wait for the zombie child to terminate it
        pid = wait3(&wstat, WNOHANG, (struct rusage *) NULL);

        //pid = 0 if child state didn't change
        //pid = 1 in case of error
        if (pid == 0 || pid == -1)
            return;
        else
            childLog();
    }
}

//write in the log file if zombie is terminated
void childLog() {
    char *msg = "Background Child Terminated\n";
    if (fileExist != 1) {
        write(fileExist, msg, strlen(msg));
    }
}