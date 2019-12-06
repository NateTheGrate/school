#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>


void error(const char *msg) {
	fprintf(stderr, "CLIENT: %s\n", msg);
	fflush(stderr);
	exit(1); 
} // Error function used for reporting issues

void error_noclose(const char *msg) {
	fprintf(stderr, "CLIENT: %s\n", msg);
	fflush(stderr);
} // Error function used for reporting issues

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
int is_valid_wrapper (char* plaintext, char* key){
    
	if(strcmp(plaintext, "") == 0 || strcmp(key, "") == 0){
		fprintf(stderr, "Error: empty file\n");
        //strcpy(error, "Error: key is too short\n");
        fflush(stderr);
		exit(1);
	}

    // check if key is correct length
    if(strlen(plaintext) > strlen(key)){
        
        fprintf(stderr, "Error: key \'%s\' is too short\n", key);
        //strcpy(error, "Error: key is too short\n");
        fflush(stderr);
		exit(1);
        return 0;
    }

    // check if characters are valid
    if(!is_valid(plaintext) || !is_valid(key)){
        error("Error: input contains bad characters\n");
        //strcpy(error, "Error: input contains bad characters\n");
        //fflush(stderr);
		//exit(1);
        return 0;
    }

    // if valid, then return true
    return 1;
}

// gets the first line a given text file
char* get_line(char* path){
	
	FILE* f = fopen(path, "r");
	if(f == NULL){
		error("Error: file does not exist");
	}

	char* buf = malloc(sizeof(char) * 10000);
	memset(buf, '\0', 10000);
	size_t linebuffer1 = 10000;
    
    getline(&buf, &linebuffer1, f);
	buf[strcspn(buf, "\n")] = '\0'; // Remove the trailing \n that fgets adds
	//printf("%s", buf);
	fclose(f);

	return buf;

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
    int size = 10000;//strlen(msg);
    char buffer[size];
    memset(buffer, '\0', size);
    strcpy(buffer, msg);
    //printf("SERVER: sending %s\n", buffer);
    // Send a Success message back to the client
	int charsRead = send(socketFD, buffer, size - 1, 0);
	if (charsRead < 0) error("ERROR writing to socket");
}



int main(int argc, char *argv[]) {
	int socketFD, portNumber;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char* buffer1;
	char* buffer2;
    
	if (argc < 4) { fprintf(stderr,"USAGE: %s plaintext key port\n", argv[0]); exit(0); } // Check usage & args

	// get path to plaintext and contents
	buffer1 = get_line(argv[1]);
	
	// get path to key and contents
	buffer2 = get_line(argv[2]);

	// check if input is valid
	if(!is_valid_wrapper(buffer1, buffer2)){
		// should error if wrong
	}

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address


	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");
	

	//char* hello = "enc";
	snd(socketFD, "enc");
	char* helloback = rec(socketFD);

	if(strncmp(helloback, "enc", 3) != 0){
		close(socketFD); // Close the socket
		error_noclose("Error: can only connect to otp_enc_d");
		exit(2);
	}

	// send plaintext and error handle
	snd(socketFD, buffer1);
	char* server_msg = rec(socketFD);
	if(strncmp(server_msg, buffer1, strlen(server_msg)) != 0){
		close(socketFD); // Close the socket
		error("Error: problem sending plaintext with server");
	}

	// send key and error handle
	snd(socketFD, buffer2);
	server_msg = rec(socketFD);

	if(strncmp(server_msg, buffer2, strlen(server_msg)) != 0){
		close(socketFD); // Close the socket
		error("Error: problem sending key with server");
	}
	
	// get encoded text from server and send it back to check if it's the same
	char* ciphertext = rec(socketFD);
	snd(socketFD, ciphertext);

	char* confirmation = rec(socketFD);
	if(strncmp(confirmation, "ok", strlen(confirmation)) != 0){
		close(socketFD);
		error("Error: problem sending confirmation with server");
	}

	// print final result
	printf("%s", ciphertext);

	close(socketFD); // Close the socket
	return 0;
}
