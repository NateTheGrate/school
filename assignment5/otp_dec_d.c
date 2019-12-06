#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

// ciphertext = encoded plaintext
// plaintext = plaintext


void error(const char *msg) {
        fprintf(stderr, "SERVER: %s\n", msg);
        fflush(stderr);
        exit(1); 
} // Error function used for reporting issues

void send_error(int connectionFD, const char *msg) { 
        int charsRead = send(connectionFD, "&", 5, 0); // Read the client's message from the socket
        if (charsRead < 0) {
            error("ERROR writing error to socket");
        }
        fprintf(stderr, "%s", msg);
        fflush(stderr);

        
} // Error function used for reporting issues to the client

// c doesn't include real modulo, so here's my implementation
int modulo(int a, int b) {

    // check for negatives from parameters
    if(b < 0) {
        return modulo(-a, -b);
    }

    int result = a % b;

    // return a % b if positive
    if(result >= 0){
        return result;
    }

    // handle negatives from a % b
    return result + b;
}

// checks if a given file contains valid characters
int is_valid(char* text){

    for(int i = 0; i < strlen(text); i++){
        // if text is not a capital letter or ' ', return false
        if(text[i] != ' ' && (text[i] < 'A' || text[i] > 'Z')){
            return 0;
        }
    }

    // return true if otherwise
    return 1;
}

// wraps is_valid to check if every file is valid
int is_valid_wrapper (char* plaintext, char* key, char* error){
    
    // check if key is correct length
    if(strlen(plaintext) > strlen(key)){
        
        //fprintf(stderr, "Error: key \'%s\' is too short\n", key);
        strcpy(error, "Error: key is too short\n");
        //fflush(stderr);
        return 0;
    }

    // check if characters are valid
    if(!is_valid(plaintext) || !is_valid(key)){
        //fprintf(stderr, "Error: input contains bad characters\n");
        strcpy(error, "Error: input contains bad characters\n");
        //fflush(stderr);
        return 0;
    }

    // if valid, then return true
    return 1;
}


void print_cipher(int* ciphertext, int length){
    
    for(int i = 0; i < length; i++){
        char temp = ciphertext[i] + 64;
        if(temp == '@') temp = ' ';
        printf("%c", temp);
    }
    printf("\n");
}

// assumes length(plaintext) <= length(key), and length = length(ciphertext)
void decrypt(char* plaintext, char* key, int* ciphertext, int length){

    for(int i = 0; i < strlen(plaintext); i++){

        // implement one-time-pad algorithm
        int diff = ( (plaintext[i] - 64) - (key[i] - 64) );
        ciphertext[i] = modulo(diff, 27);

    }

}

// assumes paramters are the same size
void convert_cipher(char* dest, int* ciphertext, int length){
    //char* dest = calloc(sizeof(char), length);
    for(int i = 0; i < length; i++){
        
        if(ciphertext[i] == 0) {
            dest[i] = ' ';
        }else{
            // convert ints to chars
            dest[i] = ciphertext[i] + 64;
        }

    }
}

char* rec(int socketFD){
    int size = 10000;
    // Get the message from the client and display it
    char* buffer = calloc(sizeof(char), size);
	memset(buffer, '\0', size);
	int charsRead = recv(socketFD, buffer, size - 1, 0); // Read the client's message from the socket
	if (charsRead < 0) error("ERROR reading from socket");
    //printf("SERVER: rec recieved %s\n", buffer);
    return buffer;

}

void snd(int socketFD, char* msg){
    int size = 10000; //strlen(msg);
    char buffer[size];
    memset(buffer, '\0', size);
    strcpy(buffer, msg);
    //printf("SERVER: sending %s\n", buffer);
    // Send a Success message back to the client
	int charsRead = send(socketFD, buffer, size - 1, 0);
	if (charsRead < 0) error("ERROR writing to socket");
}

int main(int argc, char *argv[]){

    //////////////// setup server ////////////////////////
    int listenSocketFD, establishedConnectionFD, portNumber;
	socklen_t sizeOfClientInfo;
	struct sockaddr_in serverAddress, clientAddress;

	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

    // Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

    	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

    int processnum = 0;
    while(1){
        
        // Accept a connection, blocking if one is not available until one connects
        sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
        establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept

        // create new child process to run encryption
        int spawnPid, childExitMethod, somepid;
        int* active_childs = calloc(sizeof(int), 5);
        
        // only allow up to 5 processes
        if(processnum < 5){
            spawnPid = -5;
            childExitMethod = -5;

            // start a new child process
            processnum++;
            spawnPid = fork(); 
        }else{
            
            send_error(establishedConnectionFD, "cannot allow anymore connections, please wait for one to finish\n");
        }
        
        switch(spawnPid){
                   
            case -1:
                send_error(establishedConnectionFD, "Error: could not fork process");
                exit(1);break;
            case 0:

                if (establishedConnectionFD < 0) error("ERROR on accept");
                
                // check if connected to right process
                char* init_msg = rec(establishedConnectionFD);
                if(strncmp(init_msg, "dec", 3) != 0){
                    send_error(establishedConnectionFD, "SERVER: Error: client not otp_dec\n");
                    exit(1);
                }

                // send back identifying info
                snd(establishedConnectionFD, "dec");

                
                // Get plaintext from client and send back info to make sure it's the same
                char* plaintext = rec(establishedConnectionFD);
                snd(establishedConnectionFD, plaintext);

                
                // Get key from client and send back info to make sure it's the same
                char* key = rec(establishedConnectionFD);
                snd(establishedConnectionFD, key);
                
                // do encryption
                int length = strlen(plaintext);

                int* ciphertext = calloc(sizeof(int), length);
                char* result = calloc(sizeof(char), length);
                char* err = calloc(sizeof(char), 256);

                // check if input is valid
                if(!is_valid_wrapper(plaintext, key, err)){
                    send_error(establishedConnectionFD, err);
                    exit(1);
                }else{
                    decrypt(plaintext, key, ciphertext, length);
                    //print_cipher(ciphertext, length);   
                }
                convert_cipher(result, ciphertext, length);

                // Send ciphered text back to client
                snd(establishedConnectionFD, result);

                char* confirmation = rec(establishedConnectionFD);
                if(strcmp(confirmation, result) != 0){
                    send_error(establishedConnectionFD, "Error: problem sending confirmation to client");
                    exit(1);
                }

                // else send that the final result is correct
                snd(establishedConnectionFD, "ok");

                // exit without errors
                exit(0); break;
            default:
                // check if this child processes has finished
                somepid = 0;
                somepid = waitpid(spawnPid, &childExitMethod, WNOHANG);

                // if it finishes, act like nothing happened
                if(somepid != 0){
                    processnum--;
                }else{
                    // find first open slot, and add pid to it
                    for(int i = 0; i < processnum; i++){
                        if(active_childs[i] == 0){
                            active_childs[i] = spawnPid;
                            break;
                        }
                    }
                    
                }

                // check the rest of the the unfinished processes
                for(int i = 0; i < processnum; i++){
                    somepid = 0;
                    
                    somepid = waitpid(active_childs[i], &childExitMethod, WNOHANG);

                    // if a child process finishes, set pid to 0
                    if(somepid != 0){
                        processnum--;
                        active_childs[i] = 0;
                    }else{
                        active_childs[i] = spawnPid;
                    }
                }
        }

    }
	close(establishedConnectionFD); // Close the existing socket which is connected to the client
	close(listenSocketFD); // Close the listening socket

    return 0;
}




