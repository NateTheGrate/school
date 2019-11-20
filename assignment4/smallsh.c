#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>


int exitStatus = 0; // not a boolean, 0 means good
int terminated = 0; // false
int termsignal = 0;

int fgmodeonly = 0;

void handleProcessExit(int childExitMethod){

    if(WIFEXITED(childExitMethod) != 0){
        printf("the process exited normally\n");
    }else {
        int termSignal = WTERMSIG(childExitMethod);
        printf("the process was terminated by the signal, %d\n", termSignal);
    }

}

void changeDir(char* path){
    
    if(strlen(path) == 0){
        chdir(getenv("HOME"));
        char buff[256];
        getcwd(buff, 256);
        printf("%s\n", buff);
        fflush(stdout);
        return;
    }

    if(chdir(path) != 0){
    
        printf("%s: no such file or directory\n", path);
        fflush(stdout);
        return;
    }

    char buff[256];
    getcwd(buff, 256);
    printf("%s\n", buff);
    fflush(stdout);
}

void doExit(){

    int childPid = -5;
    int childExitMethod = -5;

    while((childPid) = wait(&childExitMethod) > 0){
        // do nothing
    }
    exit(0);
}



void splitInput(char* input, char** output, int* size){

    char* word = strtok(input, " ");
    int i = 0;
    while( word != NULL ) {
        sprintf(output[i], "%s", word);
        word = strtok(NULL, " ");
        
        (*size)++;
        i++;
    }

}


void doStatus(){

    if(terminated){
        printf("terminated by signal %d\n", termsignal);
    }else{
        printf("exit value %d\n", exitStatus);
    }

    fflush(stdout);

}

void initializebghandlers(){
    struct sigaction ignore_action = {0};

    ignore_action.sa_handler = SIG_IGN;

    sigaction(SIGINT, &ignore_action, NULL);
}

void catchSIGINT(int signo){

    char* message = "terminated by SIGINT signal";
    write(STDOUT_FILENO, message, 33);
    exit(1);
}

void initializefghandlers(){
    struct sigaction SIGINT_action = {0};
    SIGINT_action.sa_handler = catchSIGINT;
    sigfillset(&SIGINT_action.sa_mask);
    SIGINT_action.sa_flags = 0;

    sigaction(SIGINT, &SIGINT_action, NULL);
}

void initializeproccesshandlers(){

    struct sigaction ignore_action = {0};

    ignore_action.sa_handler = SIG_IGN;

    sigaction(SIGTSTP, &ignore_action, NULL);

}

void handleInput(char* input, int** bg_processes, int* bgsize){

    char* dollardollarbillzyall = strstr(input, "$$");
    if(dollardollarbillzyall != NULL){
        int index = dollardollarbillzyall - input;
        char* part1 = calloc(sizeof(char), 512);
        char* part2 = calloc(sizeof(char), 512);
        char* comb = calloc(sizeof(char), 1024);
        memcpy(part1, input, index);
        memcpy(part2, &(input[index + 2]), strlen(input) - (index + 1));
        
        sprintf(comb, "%s%d%s", part1, getpid(), part2);
        input = comb;
    }

    char** results = malloc(sizeof(char*) * 512);
    int size = 0;
    for(int i = 0; i < 512; i++){
        results[i] = calloc(sizeof(char), 256);
    }

    splitInput(input, results, &size);

    if(strncmp(results[0], "exit", 4)==0 && (strlen(results[0]) >= 4)){
        doExit();
    }else if(strncmp(results[0], "cd", 2) == 0 && (strlen(results[0]) >= 2)){
        changeDir(results[1]);
    }else if(strncmp(results[0], "status", 6)==0 && (strlen(results[0]) >= 6)){
        doStatus();
    } else{
        
        int arsize = 0;
        int targetFD, sourceFD, savedout, savedin;
        savedout = dup(1);
        savedin = dup(0);

        int shouldbool = 0;
        for(int i = 0; i < size; i++){

            if(strncmp(results[i], ">", 1) == 0){

                targetFD = open(results[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (targetFD == -1) { perror("open()");}
                
                int result = dup2(targetFD, 1);
                if (result == -1) { perror("dup2");}
                fcntl(targetFD, F_SETFD, FD_CLOEXEC);
                i+=1;

            } else if(strncmp(results[i], "&", 1) == 0){
                if(!shouldbool){
                    targetFD = open("/dev/null", O_RDONLY | O_TRUNC, 0644);
                    if (targetFD == -1) { perror("open()");}
                    
                    int result = dup2(targetFD, 0);
                    if (result == -1) { perror("dup2");}
                    fcntl(targetFD, F_SETFD, FD_CLOEXEC);
                }
                i+=1;
            } else if(strncmp(results[i], "<", 1) == 0){

                sourceFD = open(results[i+1], O_RDONLY, 0644);
                if (sourceFD == -1) { perror("open()"); }
                
                int result = dup2(sourceFD, 0);
                if (result == -1) { perror("dup2"); }
                fcntl(sourceFD, F_SETFD, FD_CLOEXEC);
                shouldbool = 1;
                i+=1;

            }else {
                arsize++;
            }
            
        }

        int makebg = 0;
        char* args[arsize+1];
        for(int i = 0; i < size; i++){
            if(strncmp(results[i], ">", 1) == 0){
                i+=2;
            } else if(strncmp(results[i], "<", 1) == 0) {
                i+=2;
            } else if(strncmp(results[i], "&", 1) == 0){
                if(!fgmodeonly){
                    makebg = 1;
                }
                i+=1;
            }else {
                args[i] = results[i];
            }
        }
        args[arsize] = NULL;

        int spawnPid = -5;
        int childExitMethod = -5;
        spawnPid = fork();

        switch(spawnPid){

            case -1: 
                perror("Hull Breach!\n");
                exit(1); break;
               
            case 0:
                if(makebg == 1){
                    
                    (*bg_processes)[*bgsize] = getpid();
                    
                    (*bgsize)++;
                    initializebghandlers();
                }else{
                    initializefghandlers();
                }
                initializeproccesshandlers();

                execvp(*args, args);
                perror(args[0]);
                exit(1); break;
            
            default:
                if(!makebg){
                    waitpid(spawnPid, &childExitMethod, 0);

                    if (WIFEXITED(childExitMethod)){
                        exitStatus = WEXITSTATUS(childExitMethod);
                        terminated = 0;
                    }else{
                        terminated = 1;
                        termsignal = WTERMSIG(childExitMethod);
                    }

                }else{
    
                    (*bg_processes)[*bgsize] = spawnPid;
                    
                    (*bgsize)++;
                }
                dup2(savedin, 0);
                dup2(savedout, 1);
                close(savedin);
                close(savedout);
                
        }
        
    }
}

void check_bg(int* bgprocesses, int* size){

    int childPid = bgprocesses[0];
    int childExitMethod = -5;

    for(int i = 0; i < *size; i++){

        childPid = bgprocesses[i];
        
        if(childPid != -1){
            int actualPid = waitpid(childPid, &childExitMethod, WNOHANG);

            if(actualPid != 0){

                int myexitStatus = 0;
                int myterminated = 0;
                int mytermsignal = 0;

                if (WIFEXITED(childExitMethod)){
                    myexitStatus = WEXITSTATUS(childExitMethod);
                    myterminated = 0;
                }else{
                    myterminated = 1;
                    mytermsignal = WTERMSIG(childExitMethod);
                }
                
                printf("background pid %d is done: ", actualPid);
                fflush(stdout);
                if(terminated){
                    printf("terminated by signal %d\n", termsignal);
                }else{
                    printf("exit value %d\n", exitStatus);
                }

                fflush(stdout);
                bgprocesses[i] = -1;
            }
        }
        
    }

}


void catchSIGTSTP(int signo)
{
    char* message;
    
    if(!fgmodeonly){
        message = "\nEnterting foreground-only mode\n";
        fgmodeonly = !fgmodeonly;
    }else{
        message = "\nExiting foregroud-only mode\n";
        fgmodeonly = !fgmodeonly;
    }
    write(STDOUT_FILENO, message, 33);
}

void main()
{

    struct sigaction ignore_action = {0};

    ignore_action.sa_handler = SIG_IGN;

    sigaction(SIGINT, &ignore_action, NULL);

    struct sigaction SIGTSTP_action = {0};
    SIGTSTP_action.sa_handler = catchSIGTSTP;
    sigfillset(&SIGTSTP_action.sa_mask);

    sigaction(SIGTSTP, &SIGTSTP_action, NULL);

  int numCharsEntered = -5; // How many chars we entered
  int currChar = -5; // Tracks where we are when we print out every char
  size_t bufferSize = 0; // Holds how large the allocated buffer is
  char* lineEntered = NULL; // Points to a buffer allocated by getline() that holds our entered string + \n + \0


  int* bgprocesses = calloc(sizeof(int), 10000);
  int bgsize = 0;

  while(1)
  {
    // Get input from the user
    while(1)
    {
      printf(": ");
      fflush(stdout);
      // Get a line from the user
      numCharsEntered = getline(&lineEntered, &bufferSize, stdin);
      if (numCharsEntered == -1)
        clearerr(stdin);
      else
        break; // Exit the loop - we've got input
    }

    // Remove the trailing \n that getline adds
    lineEntered[strcspn(lineEntered, "\n")] = '\0';

    if(lineEntered[0] != '#'){
        handleInput(lineEntered, &bgprocesses, &bgsize);
    }
    check_bg(bgprocesses, &bgsize);

    // Free the memory allocated by getline() or else memory leak
    free(lineEntered);
    lineEntered = NULL;
  }
}