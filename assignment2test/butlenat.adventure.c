#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <assert.h>


struct player{
    char* history;
    int size;
};

struct connections{
    char* names;
    int size;
    char name;
};

int resultInt;
pthread_t p;
pthread_mutex_t lock;


char* newest_dir(){

    int newestDirTime = -1; // Modified timestamp of newest subdir examined
    char targetDirPrefix[32] = "butlenat.rooms."; // Prefix we're looking for
    char* newestDirName = malloc(256 * sizeof(char)); // Holds the name of the newest dir that contains prefix
    memset(newestDirName, '\0', sizeof(newestDirName));

    DIR* dirToCheck; // Holds the directory we're starting in
    struct dirent *fileInDir; // Holds the current subdir of the starting dir
    struct stat dirAttributes; // Holds information we've gained about subdir

    dirToCheck = opendir("."); // Open up the directory this program was run in

    if (dirToCheck > 0) // Make sure the current directory could be opened
    {
        while ((fileInDir = readdir(dirToCheck)) != NULL) // Check each entry in dir
        {
            if (strstr(fileInDir->d_name, targetDirPrefix) != NULL) // If entry has prefix
            {
                //printf("Found the prefex: %s\n", fileInDir->d_name);
                stat(fileInDir->d_name, &dirAttributes); // Get attributes of the entry

                if ((int)dirAttributes.st_mtime > newestDirTime) // If this time is bigger
                {
                    newestDirTime = (int)dirAttributes.st_mtime;
                    memset(newestDirName, '\0', sizeof(newestDirName));
                    strcpy(newestDirName, fileInDir->d_name);
                    //printf("Newer subdir: %s, new time: %d\n",
                    //        fileInDir->d_name, newestDirTime);
                }
            }
        }
    }

    closedir(dirToCheck); // Close the directory we opened

    //printf("Newest entry found is: %s\n", newestDirName);

    return newestDirName;
}

char* find_id(char* path, char* id){
    DIR* d = opendir(path);
    struct dirent* dir;
    if(d){

         while ((dir = readdir(d)) != NULL){

            char* buffer = malloc(256*sizeof(char));
            sprintf(buffer, "%s/%s", path, dir->d_name);

            FILE* f = fopen(buffer, "r");
            
            if(f == NULL){

                printf("no such file");
                exit(1);

            }
            
            char buf[256];
            while (fscanf(f,"%s",buf)==1) {
                //printf("%s == %s", buf, "START_ROOM");
                if (strcmp(buf, id) == 0){
                    return buffer;
                }
            }
            fclose(f);

        }

        closedir(d);

    }

    //return NULL;
}


void print_connections(struct connections* cons){



    printf("name: %c\n", cons->name);
    printf("connections: ");
    int i;
    for(i = 0; i < cons->size; i++){
        printf("%c, ", cons->names[i]);
    }
    printf("\n");

}

struct connections* get_connections(char* path){
    FILE* f = fopen(path, "r");
    struct connections* cons = malloc(sizeof(cons));
    cons->size = 0;
    cons->names = malloc(6 * sizeof(char));
    if(f == NULL){

        printf("no such file");
        exit(1);

    }

    size_t linebuffer = 256;
    char* buf = (char*)malloc(linebuffer * sizeof(char));
    int line_size = getline(&buf, &linebuffer, f);
    char* comp = "CONNECTION ";
    while(line_size >= 0){
        
        comp = "ROOM NAME: ";
        if(strstr(buf, comp) != NULL){
            cons->name = buf[strlen(buf) - 2];
        }

        comp = "CONNECTION ";
        if(strstr(buf, comp) != NULL){
            cons->names[cons->size] = buf[strlen(buf) - 2];
            cons->size++;
        }
        line_size = getline(&buf, &linebuffer, f);
    }
    fclose(f);
    //print_connections(cons);
    return cons;
}

void create_file(char* path){

    int file_desc = open(path, O_WRONLY | O_CREAT, 0600);
    if(file_desc < 0){
        fprintf(stderr, "could not open %s\n", path);
        //perrer("error in create_file()");
        exit(1);
    }
    close(file_desc);
}

void write_to_file(char* path, char* data){
    int file_desc = open(path, O_WRONLY);
    if(file_desc < 0){
        fprintf(stderr, "could not open %s\n", path);
        //perrer("error in write_to_file()");
        exit(1);
    }

    write(file_desc, data, strlen(data) * sizeof(char));

    close(file_desc);
}

void* write_time(){

    time_t t;
    char* buffer = malloc(sizeof(char) * 256);

    struct tm* pnt;
    time(&t);

    pnt = localtime(&t);

    strftime(buffer, sizeof(char) * 256, "%I:%M%p, %A, %B %d, %C%y", pnt);

    create_file("./currentTime.txt");
    write_to_file("./currentTime.txt", buffer);
 
}

void get_time(){

    FILE* f = fopen("./currentTime.txt", "r");
    size_t linebuffer = 256;
    char* buf = (char*)malloc(linebuffer * sizeof(char));
    getline(&buf, &linebuffer, f);
    printf("%s", buf);
}

char* get_name_input(){

    char* buffer = malloc(256 * sizeof(char));
    size_t line_size =256;
    getline(&buffer, &line_size, stdin);

    return buffer;

}

void print_stats(struct player* p){

    printf("YOU TOOK %d STEPS. YOU PATH TO VICTORY WAS:\n", p->size);
    int i;
    for(i = 0; i < p->size; i++){
        printf("%c\n", p->history[i]);
    }

}


void create_thread(){

    pthread_mutex_lock(&lock);
    int resultInt;
    pthread_t p;

    resultInt = pthread_create(&p, NULL, write_time, NULL);
    assert(0 == resultInt);


    void* val;

    resultInt = pthread_join(p, &val);
    assert(0 == resultInt);

    pthread_mutex_unlock(&lock);

}

void* game(){


    

    struct player* p = malloc(sizeof(struct player));
    p->history = malloc(sizeof(char) * 256);
    p->size = 0;

    char* path = find_id(newest_dir(), "START_ROOM");
    struct connections* cons = get_connections(path);


    while(strcmp(path, find_id(newest_dir(), "END_ROOM") )){


        
        printf("CURRENT LOCATION: %c\n", cons->name);
        printf("POSSIBLE CONNENCTIONS: ");
        int i;
        for(i = 0; i < cons->size - 1; i++){
            printf("%c, ", cons->names[i]);
        }
        printf("%c.\n", cons->names[cons->size - 1]);
        printf("WHERE TO? >");

        char* input = get_name_input();
        // check for time?

        while (strncmp(input, "time", 4) == 0){
            printf("\n");
            create_thread();
            
            get_time();
            printf("\n");


            printf("\nWHERE TO? >");

            input = get_name_input();
        }

        int isroom = 0;

        for(i = 0; i < cons->size; i++){

            if(input[0] == cons->names[i]){
                isroom = 1;
                break;
            }else{
                isroom = 0;
            }
        }
        if(isroom == 0){
            printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN\n");
        }else{
            path[strlen(path) - 5] = input[0];
            p->history[p->size] = input[0];
            p->size++;
        }
        

        
        cons = get_connections(path);
    
        printf("\n");
    }
    
    printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!");
    print_stats(p);

    return NULL;
}



int main(){

    if (pthread_mutex_init(&lock, NULL) != 0){ 

        printf("mutex init has failed"); 
        return 1; 
    
    } 

    resultInt = pthread_create(&p, NULL, game, NULL);
    assert(0 == resultInt);

    void* val;

    resultInt = pthread_join(p, &val);
    assert(0 == resultInt);

    pthread_mutex_destroy(&lock);
    return 0;
}