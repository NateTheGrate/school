#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

typedef int bool;
#define true 1
#define false 0

const enum room_ids {START_ROOM, MID_ROOM, END_ROOM};
const char room_names[10] = {'a','b','c','d','e','f','g','h','i','j'};


struct room {
    enum room_ids id;
    char name;
    char* connections;
    int num_connections;
};
 

char* make_directory(){
    char* buffer = calloc(21, sizeof(char));
    sprintf(buffer, "%s.%d", "butlenat.rooms", getpid());
    mkdir(buffer, 0755);

    return buffer;
}

int random_num(int used[10], int size){
    if(used == NULL){
        return -1;
    }
    
    int r = rand() % 10;
    int comp = used[0];

    bool stop = false;
    while(stop == false){

        r = rand() % 10;

        for(int i = 0; i < size; i++){
            if (r == used[i]){
                stop = false;
                break;
            }else {
                stop = true;
            }
        }
    }

    return r;

}

struct room* initialize_rooms(){
    struct room* rooms = malloc(7 * sizeof(struct room));

    int nums_used[10];
    int size = 0;

    int r = (rand() % 10);
    nums_used[0] = r;
    size++;

    rooms[0].id = START_ROOM;
    rooms[0].name = room_names[r];
    rooms[0].num_connections = 3 + (rand() % 3);
    rooms[0].connections = malloc(rooms[0].num_connections * sizeof(char));
    for(int i = 1; i < 6; i++){
        r = random_num(nums_used, size);

        rooms[i].id = MID_ROOM;
        rooms[i].name = room_names[r];
        rooms[i].num_connections = 3 + (rand() % 3);
        rooms[i].connections = malloc(rooms[0].num_connections * sizeof(char));

        nums_used[i] = r;
        size++;
    }
    rooms[6].id = END_ROOM;
    rooms[6].name = room_names[random_num(nums_used, size)];
    rooms[6].num_connections = 3 + (rand() % 3);
    rooms[6].connections = malloc(rooms[0].num_connections * sizeof(char));

    return rooms;
}

void print_rooms(struct room* rooms){
    for(int i = 0; i < 7; i++){
        printf("room id: %d, room name: %c, num of connections: %d\n", rooms[i].id, rooms[i].name, rooms[i].num_connections);
    }
}


// Returns true if all rooms have 3 to 6 outbound connections, false otherwise
/**bool IsGraphFull()  
{

}

// Adds a random, valid outbound connection from a Room to another Room
void AddRandomConnection()  
{
  struct room A;  // Maybe a struct, maybe global arrays of ints
  struct room B;

  while(true)
  {
    A = GetRandomRoom();

    if (CanAddConnectionFrom(A) == true)
      break;
  }

  do
  {
    B = GetRandomRoom();
  }
  while(CanAddConnectionFrom(B) == false || IsSameRoom(A, B) == true || ConnectionAlreadyExists(A, B) == true);

  ConnectRoom(A, B);  // TODO: Add this connection to the real variables, 
  ConnectRoom(B, A);  //  because this A and B will be destroyed when this function terminates
}

// Returns a random Room, does NOT validate if connection can be added
struct room GetRandomRoom()
{

}

// Returns true if a connection can be added from Room x (< 6 outbound connections), false otherwise
bool CanAddConnectionFrom(struct room x) 
{

}
// Returns true if a connection from Room x to Room y already exists, false otherwise
bool ConnectionAlreadyExists(x, y)
{

}

// Connects Rooms x and y together, does not check if this connection is valid
void ConnectRoom(struct room x, struct room y) 
{

}

// Returns true if Rooms x and y are the same Room, false otherwise
bool IsSameRoom(struct room x, struct room y) 
{

}**/

int main(){

    srand(time(NULL)); 

    char* directory = make_directory();
    printf("directory: %s\n", directory);

    struct room* rooms = initialize_rooms();
    
    print_rooms(rooms);


    // Create all connections in graph
    /**while (IsGraphFull() == false)
    {
        AddRandomConnection();
    }**/

    free(directory);
    return 0;
}