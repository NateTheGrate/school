#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

char* id_names[3] = {"START_ROOM", "MID_ROOM", "END_ROOM"};
const char possible_names[10] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j'};

struct room {

    char* id;
    char name;
    int num_connections;
    char* connections;

};

void init_room(struct room* A, int myID, int myName){

    A->id = id_names[myID];
    A->name = possible_names[myName];
    A->num_connections = 0;
    A->connections = malloc(6 * sizeof(char));

}

int random_index(int used[10], int size){
    if(used == NULL){
        return -1;
    }

    int r = rand() % 10;
    int compare = used[0];

    int stop = 0;
    while(!stop){

        r = rand() % 10;

        int i;
        for(i = 0; i < size; i++){

            if(r == used[i]){
                stop = 0;
                break;

            }else{
                stop = 1;
            }
        }
    }

    return r;
}

struct room* init_room_array(){

    struct room* rooms = malloc(7 * sizeof(struct room));
    
    int used_nums[10];
    int size = 0;

    int r = rand() % 10;
    used_nums[0] = r;
    size++;

    init_room(&(rooms[0]), 0, r);
    int i;
    for(i = 1; i < 6; i++){

        r = random_index(used_nums, size);
        used_nums[size] = r;
        size++;

        init_room(&(rooms[i]), 1, r);

    } 

    init_room(&(rooms[6]), 2, random_index(used_nums, size));

    return rooms;


}

void print_room(struct room* room){
    printf("id: %s, name: %c, number of connection: %d", \
            room->id, room->name, room->num_connections);

    printf(" connections: ");
    int i;
    for(i = 0; i < room->num_connections; i++){
        printf("%c ", room->connections[i]);
    }
    printf("\n");
    
}

void print_rooms(struct room** rooms){

    int i;
    for(i = 0; i < 7; i++){

        print_room(&((*rooms)[i]));

    }

}

// Returns a random Room, does NOT validate if connection can be added
struct room* GetRandomRoom(struct room** rooms)
{
    return &((*rooms)[rand() %7]);
}

// Returns true if a connection can be added from Room x (< 6 outbound connections), false otherwise
int CanAddConnectionFrom(struct room* x) 
{
    if(x->num_connections >= 6){
        return 0;
    }
    return 1;
}

// Returns true if Rooms x and y are the same Room, false otherwise
int IsSameRoom(struct room* x, struct room* y) 
{
    if(x->name == y->name){
        return 1;
    }
    return 0;
}

// Returns true if a connection from Room x to Room y already exists, false otherwise
int ConnectionAlreadyExists(struct room* x, struct room* y)
{
    int i;
    for(i = 0; i < x->num_connections; i++){

        if(x->connections[i] == y->name){
            return 1;
        }

    }

    return 0;
}

// Connects Rooms x and y together, does not check if this connection is valid
void ConnectRoom(struct room* x, struct room* y) 
{
    x->connections[x->num_connections] = y->name;
    x->num_connections++;
}

// Returns true if all rooms have 3 to 6 outbound connections, false otherwise
int IsGraphFull(struct room** rooms)  
{
    int i;
    for(i = 0; i < 7; i++){
        if((*rooms)[i].num_connections < 3 ){
            return 0;
        }
    }

    return 1;
}

// Adds a random, valid outbound connection from a Room to another Room
void AddRandomConnection(struct room** rooms) 
{
    struct room* A;  // Maybe a struct, maybe global arrays of ints
    struct room* B;

    while(1)
    {
        A = GetRandomRoom(rooms);

        if (CanAddConnectionFrom(A)){
            break;
        }
    }

    do
    {
        B = GetRandomRoom(rooms);
    }
    while(CanAddConnectionFrom(B) == 0 || IsSameRoom(A, B) == 1 || ConnectionAlreadyExists(A, B) == 1);

    ConnectRoom(A, B);  // TODO: Add this connection to the real variables, 
    ConnectRoom(B, A);  //  because this A and B will be destroyed when this function terminates
}

char* make_directory(){
    char* buffer = calloc(21, sizeof(char));
    sprintf(buffer, "%s.%d", "butlenat.rooms", getpid());
    mkdir(buffer, 0755);

    return buffer;
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
    int file_desc = open(path, O_WRONLY | O_APPEND);
    if(file_desc < 0){
        fprintf(stderr, "could not open %s\n", path);
        //perrer("error in write_to_file()");
        exit(1);
    }

    write(file_desc, data, strlen(data) * sizeof(char));

    close(file_desc);
}

char* room_to_string(struct room* room){
    char* buffer = malloc(256 * sizeof(char));

    sprintf(buffer, "ROOM NAME: %c\n", room->name);
    int i;
    for(i = 0; i < room->num_connections; i++){
        sprintf(buffer + strlen(buffer), "CONNECTION %d: %c\n", i, room->connections[i]);
    }
    sprintf(buffer + strlen(buffer), "ROOM_TYPE: %s\n", room->id);
    //sprintf(buffer + strlen(buffer), "\n");

    return buffer;

}


void write_rooms(struct room** rooms, char* dir){

    struct room* rs = *rooms;
    int i;
    for(i = 0; i < 7; i++){

        char* filepath = malloc(sizeof(char) * 256);
        sprintf(filepath, "%s/%c.txt", dir, rs[i].name);

        char* data = room_to_string(&(rs[i]));

        write_to_file(filepath, data);

    }

}

int main(){
    srand(time(NULL));

    struct room* rooms = init_room_array();

    // Create all connections in graph
    while (IsGraphFull(&rooms) == 0)
    {
        AddRandomConnection(&rooms);
    }

    char* dir = make_directory();

    int i;
    for(i = 0; i < 7; i++){
        char buffer[256];
        sprintf(buffer, "%s/%c.txt", dir,rooms[i].name);
        create_file(buffer);
    }

    write_rooms(&rooms, dir);

    return 0;
}