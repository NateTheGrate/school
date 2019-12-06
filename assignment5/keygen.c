#include <stdio.h> 
#include <stdlib.h> 
#include <time.h>

int main(int argc, char* argv[]){

    if (argc < 2) { fprintf(stderr,"USAGE: %s keylength\n", argv[0]); exit(1); } // Check usage & args

    int length = atoi(argv[1]);

    srand(time(NULL)); 
    for(int i = 0; i < length; i++){
        char r = rand() % 27 + 64;
        if(r == '@') r = ' ';
        printf("%c", r);
    }
    printf("\n");

    return 0;
}