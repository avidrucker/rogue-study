#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* DESCRIPTION OF STUDY

This version of the study focuses on creating rooms in a 3x3 grid, and then, as in
maptest1, connecting them with rectangular corridors. This version is otherwise
identical to the first version of the study.

*/

// DONE: fix bug where sometimes rooms are not all connected within 50 epochs
// DONE: feat conversion of room-corridor intersections to doors
// DONE: figure out which room is the "first room" and place player there
// DONE: place the exit in the "last room" (the room farthest from the first room)
// TODO: add a win condition when the player reaches the exit
// TODO: find a room that isn't the first nor last room that is only connected to 
//       one other room, and mark this as the secret room to put treasure in, if no
//       such room exists, then swap the last room with 2nd to last room and make
//       the last room now be the secret room. eg: rooms 2->7->0->8->4 and 0->3 also.
//       In this example, room 2 is the first room, room 3 is the secret room, and 
//       room 4 is the last room. If there are two rooms or more that are only 
//       connected to one other room, then randomly choose one of them to be the 
//       secret room.
// DONE: fix bug where corridors sometimes lead to dead ends (remove all dead ends)

struct Rectangle
{
    int xPos;
    int yPos;
    int width;
    int height;
};

struct Point {
    int x; // column, starting at the left w/ col 0
    int y; // row, starting at the top w/ row 0
};

int ROWS = 30;
int COLS = 60;
int ROOM_COUNT = 9;
int MAX_CORRIDOR_EPOCHS = 50;
int MAX_ROOM_EPOCHS = 10;
int MAX_DOORS = 20;
int CORRIDOR_COUNT = 15;
char PLAYER_CHAR = 'P';
char EXIT_CHAR = 'E';
char TREASURE_CHAR = 'T';
// int debug_once = 1;
int randSeed = 0; // NULL means random

void fillMatrix(char matrix[][COLS], int rows, int cols, char input)
{
    int i, j;

    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < cols; j++)
        {
            matrix[i][j] = input;
        }
    }
}

void printMatrix(char matrix[][COLS], int rows, int cols)
{
    int i, j;

    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < cols; j++)
        {
            printf("%c ", matrix[i][j]);
        }
        printf("\n");
    }
}

int pointIsDoorOrFloor(char matrix[][COLS], struct Point point) {
    if (matrix[point.y][point.x] == '%' || matrix[point.y][point.x] == '.' || 
            matrix[point.y][point.x] == '*') {
        return 1;
    }
    return 0;
}

struct Point destinationPoint(struct Point playerPos, char direction)
{
    struct Point destination = playerPos;
    if (direction == 'w')
    {
        destination.y--;
    }
    else if (direction == 'a')
    {
        destination.x--;
    }
    else if (direction == 's')
    {
        destination.y++;
    }
    else if (direction == 'd')
    {
        destination.x++;
    }
    return destination;
}


int rectCollision(struct Rectangle a, struct Rectangle b)
{
    if (a.xPos <= b.xPos + b.width &&
        a.xPos + a.width >= b.xPos &&
        a.yPos <= b.yPos + b.height &&
        a.yPos + a.height >= b.yPos)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int pointInRect(struct Rectangle rect, struct Point point) {
    if (point.x >= rect.xPos && point.x < rect.xPos + rect.width &&
        point.y >= rect.yPos && point.y < rect.yPos + rect.height) {
        return 1;
    }
    return 0;
}

void placeRoom(char matrix[][COLS], int x, int y, int width, int height, char wallChar)
{
    int i, j;

    for (i = x; i < x + width; i++)
    {
        for (j = y; j < y + height; j++)
        {
            if (i == x || i == x + width - 1 || j == y || j == y + height - 1)
            {
                matrix[j][i] = wallChar;
            }
            else
            {
                matrix[j][i] = '.';
            }
        }
    }
}

struct Rectangle *placeRooms(char matrix[][COLS]) {
    // srand(time(NULL)); // Seed the random number generator

    int numRooms = rand() % 5 + 5; // Random number of rooms between 5 and 9
    struct Rectangle *rooms = malloc(numRooms * sizeof(struct Rectangle));
    int placed = 0;
    int quadrantUsed[9] = {0}; // To track used quadrants

    while (placed < numRooms) {
        int quadrant = rand() % 9;
        while (quadrantUsed[quadrant]) { // Find an unused quadrant
            quadrant = (quadrant + 1) % 9;
        }

        int thirdWidth = COLS / 3;
        int thirdHeight = ROWS / 3;

        int x = quadrant % 3 * thirdWidth + 1; // Calculate quadrant position
        int y = quadrant / 3 * thirdHeight + 1;

        int maxWidth = thirdWidth - 4; // 2 tiles less than a third of the map's dimensions
        int maxHeight = thirdHeight - 4;

        int width = rand() % (maxWidth - 5) + 5; // Room size between 5x5 and maxWidth x maxHeight
        int height = rand() % (maxHeight - 5) + 5;

        struct Rectangle c = {x, y, width, height};
        placeRoom(matrix, x, y, width, height, '0' + placed); // Place room on map
        rooms[placed] = c;
        quadrantUsed[quadrant] = 1; // Mark this quadrant as used
        placed++;
    }
    
    return rooms;
}

void redrawAllRooms(char matrix[][COLS], struct Rectangle *rooms, int numRooms) {
    for (int i = 0; i < numRooms; i++) {
        // placeRoom(matrix, rooms[i].xPos+1, rooms[i].yPos+1, rooms[i].width-2, rooms[i].height-2, i + '0');
        placeRoom(matrix, rooms[i].xPos, rooms[i].yPos, rooms[i].width, rooms[i].height, i + '0');
    }
}

// TODO: fix bug where room returned is not always the topmost, leftmost room
struct Rectangle findTopLeftRoom(struct Rectangle *rooms, int numRooms) {
    if(numRooms == 0) {
        printf("Error: no rooms, cannot locate top left room\n");
        struct Rectangle noRoom = {-1, -1, -1, -1};
        return noRoom;
    }

    struct Rectangle topLeftRoom = rooms[0];
    int minDistance = rooms[0].xPos + rooms[0].yPos;

    for (int i = 1; i < numRooms; i++) {
        int distance = rooms[i].xPos + rooms[i].yPos;
        if (distance < minDistance) {
            topLeftRoom = rooms[i];
            minDistance = distance;
        }
    }

    printf("The top left room is at (%d, %d)\n", topLeftRoom.xPos, topLeftRoom.yPos);

    return topLeftRoom;
}

int getRoomIndexFromRect(struct Rectangle rect, struct Rectangle *rooms, int numRooms) {
    for (int i = 0; i < numRooms; i++) {
        if (rooms[i].xPos == rect.xPos && rooms[i].yPos == rect.yPos &&
            rooms[i].width == rect.width && rooms[i].height == rect.height) {
            return i;
        }
    }
    return -1;
}

// a debugging function
void printRoomDetails(char matrix[][COLS], struct Rectangle *rooms, int roomIndex) {
    if(roomIndex == -1) {
        printf("Error: room not found, cannot print room details\n");
        return;
    }
    struct Rectangle room = rooms[roomIndex];
    printf("Room %d details:\n", roomIndex);
    printf("x: %d\n", room.xPos);
    printf("y: %d\n", room.yPos);
    printf("width: %d\n", room.width);
    printf("height: %d\n", room.height);
    printf("char at (x: %d, y: %d): %c\n", room.xPos+1, room.yPos+1, matrix[room.yPos+1][room.xPos+1]);
}


void dfs(int room, int numRooms, int connections[][numRooms], int visited[]) {
    visited[room] = 1;

    for (int i = 0; i < numRooms; i++) {
        if (connections[room][i] && !visited[i]) {
            dfs(i, numRooms, connections, visited);
        }
    }
}

int isFullyTransitive(int numRooms, int connections[][numRooms]) {
    int visited[numRooms];
    for (int i = 0; i < numRooms; i++) {
        visited[i] = 0;
    }

    // Start DFS from the first room
    dfs(0, numRooms, connections, visited);

    // Check if all rooms were visited
    for (int i = 0; i < numRooms; i++) {
        if (!visited[i]) {
            return 0; // Not fully transitive
        }
    }

    return 1; // Fully transitive
}


int farthestRoom(int startRoomIndex, int numRooms, int connections[][numRooms]) {
    int distances[numRooms];
    for (int i = 0; i < numRooms; i++) {
        distances[i] = -1;
    }
    distances[startRoomIndex] = 0;

    int queue[numRooms];
    int front = 0, back = 0;
    queue[back++] = startRoomIndex;

    while (front != back) {
        int currentRoom = queue[front++];
        for (int i = 0; i < numRooms; i++) {
            if (connections[currentRoom][i] && distances[i] == -1) {
                distances[i] = distances[currentRoom] + 1;
                queue[back++] = i;
            }
        }
    }

    int maxDistance = -1, farthestRoomIndex = -1;
    for (int i = 0; i < numRooms; i++) {
        if (distances[i] > maxDistance) {
            maxDistance = distances[i];
            farthestRoomIndex = i;
        }
    }

    return farthestRoomIndex;
}


struct Rectangle clipCorridor(struct Rectangle room1, struct Rectangle room2, struct Rectangle corridor, int isTall) {
    struct Rectangle topRoom, bottomRoom, leftRoom, rightRoom;

    // if(debug_once) {
    //     printf("Clipping the first corridor w/ room1 and room2\n");
    //     if(isTall) {
    //         printf("Corridor is tall\n");
    //     } else {
    //         printf("Corridor is wide\n");
    //     }
    //     printf("room1: x: %d, y: %d, width: %d, height: %d\n", room1.xPos, room1.yPos, room1.width, room1.height);
    //     printf("room2: x: %d, y: %d, width: %d, height: %d\n", room2.xPos, room2.yPos, room2.width, room2.height);
    //     printf("corridor: x: %d, y: %d, width: %d, height: %d\n", corridor.xPos, corridor.yPos, corridor.width, corridor.height);
    // }

    // Determine which room is on top/bottom or left/right
    if (room1.yPos < room2.yPos) {
        topRoom = room1;
        bottomRoom = room2;
    } else {
        topRoom = room2;
        bottomRoom = room1;
    }

    if (room1.xPos < room2.xPos) {
        leftRoom = room1;
        rightRoom = room2;
    } else {
        leftRoom = room2;
        rightRoom = room1;
    }

    if (isTall) {
        // Clip the corridor vertically
        corridor.yPos = topRoom.yPos + topRoom.height - 1;
        corridor.height = bottomRoom.yPos - corridor.yPos + 1;
        if(corridor.height < 3) {
            corridor.height = 3;
        }
    } else {
        // Clip the corridor horizontally
        corridor.xPos = leftRoom.xPos + leftRoom.width - 1;
        corridor.width = rightRoom.xPos - corridor.xPos + 1;
        if(corridor.width < 3) {
            corridor.width = 3;
        }
    }

    // if(debug_once) {
    //     printf("Clipped corridor: x: %d, y: %d, width: %d, height: %d\n", corridor.xPos, corridor.yPos, corridor.width, corridor.height);
    //     debug_once = 0;
    // }

    return corridor;
}


// TODO: reject corridors that intersect other corridors
// DONE: modify placeCorridors so that it has enough corridors to connect each room to at least one other room,
//       and that each room is reachable (i.e. there are no isolated rooms), this will involve removing param 'int numCorridors' 
// TODO: modify placeCorridors so that two corridors can be placed down, one that is horizontal and one that is vertical
//       to connect two rooms via two intersecting corridors where both corridors start at a room wall and end at a room wall
// DONE: modify placeCorridors so that each room is connected to at least one other room
// TODO: modify placeCorridors so that each door to a room (a corridor-room intersection) does not occur at a room corner
struct Rectangle *placeCorridors(char matrix[][COLS], struct Rectangle *rooms, int numRooms, int numCorridors, int connections[][numRooms]) {
    int placed = 0;
    int iterationCount = 0;
    int epoch = 0;
    // save initial matrix state so if we need to redo the hallways we can by looping over matrix
    char initialMatrix[ROWS][COLS];
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            initialMatrix[i][j] = matrix[i][j];
        }
    }

    // debugging message
    // printf("Initial matrix fully transitive: %d\n", isFullyTransitive(numRooms, connections));

    struct Rectangle *corridors = malloc(numCorridors * sizeof(struct Rectangle));
    while (isFullyTransitive(numRooms, connections) == 0 && placed < numCorridors) {
        int isTall = rand() % 2; // 0 for wide, 1 for tall
        int width = isTall ? 3 : rand() % 10 + 7;
        int height = isTall ? rand() % 10 + 7 : 3;
        int x = rand() % (COLS - width - 2) + 2;
        int y = rand() % (ROWS - height - 2) + 2;
        struct Rectangle corridor = {x, y, width, height};

        int intersectedRooms[3]; // changed to 3 to include the case where the corridor intersects more than 2 rooms
        int corrRoomIntersections = 0;
        for (int i = 0; i < numRooms; i++) {
            // struct Rectangle shrunkCorridor = {corridor.xPos+1, corridor.yPos+1, corridor.width-2, corridor.height-2};
            // TODO: test shrinking corridors to see if this helps w/ intersections
            // printf("shrunken corridor dimensions: x: %d, y: %d, width: %d, height: %d\n", shrunkCorridor.xPos, shrunkCorridor.yPos, shrunkCorridor.width, shrunkCorridor.height);
            struct Rectangle shrunkRoom = {rooms[i].xPos+2, rooms[i].yPos+2, rooms[i].width-4, rooms[i].height-4};
            if (rectCollision(corridor, shrunkRoom)) {
                if(corrRoomIntersections > 2) {
                    // printf("Error: corridor intersects more than 2 rooms\n");
                    break;
                } else {
                    intersectedRooms[corrRoomIntersections] = i;
                    corrRoomIntersections++;
                }
            }
        }

        if (corrRoomIntersections == 2) {
            // Check if the rooms are already connected
            if (connections[intersectedRooms[0]][intersectedRooms[1]]) {
                // The rooms are already connected, so skip this corridor
                // printf("Rooms %d and %d are already connected, skipping adding this corridor\n", intersectedRooms[0], intersectedRooms[1]);
            } else {
                // check to confirm that all 4 corners of the corridor are inside a room
                struct Point topLeft = {corridor.xPos, corridor.yPos};
                struct Point topRight = {corridor.xPos + corridor.width - 1, corridor.yPos};
                struct Point bottomLeft = {corridor.xPos, corridor.yPos + corridor.height - 1};
                struct Point bottomRight = {corridor.xPos + corridor.width - 1, corridor.yPos + corridor.height - 1};
                int topLeftInRoom = pointInRect(rooms[intersectedRooms[0]], topLeft) || pointInRect(rooms[intersectedRooms[1]], topLeft);
                int topRightInRoom = pointInRect(rooms[intersectedRooms[0]], topRight) || pointInRect(rooms[intersectedRooms[1]], topRight);
                int bottomLeftInRoom = pointInRect(rooms[intersectedRooms[0]], bottomLeft) || pointInRect(rooms[intersectedRooms[1]], bottomLeft);
                int bottomRightInRoom = pointInRect(rooms[intersectedRooms[0]], bottomRight) || pointInRect(rooms[intersectedRooms[1]], bottomRight);
                if(topLeftInRoom && topRightInRoom && bottomLeftInRoom && bottomRightInRoom) {
                    // Place the corridor
                    // printf("Placing corridor %c\n", placed + 'a');
                    // Clip the corridor so it doesn't clip through the rooms
                    corridor = clipCorridor(rooms[intersectedRooms[0]], rooms[intersectedRooms[1]], corridor, isTall);
                    placeRoom(matrix, corridor.xPos, corridor.yPos, corridor.width, corridor.height, placed + 'a'); // note: adding + 'a' converts int to char, starting at 'a'
                    corridors[placed] = corridor;
                    placed++;
                    // Update the connections matrix
                    connections[intersectedRooms[0]][intersectedRooms[1]] = 1;
                    connections[intersectedRooms[1]][intersectedRooms[0]] = 1;
                } else {
                    // printf("Error: corridor intersects 2 rooms but not all 4 corners are inside a room\n");
                }
            }
        }
        iterationCount++;

        if(epoch > MAX_CORRIDOR_EPOCHS) {
            //// TODO: trigger a new map generation because the current room setup seems
            //         to make placing corridors difficult
            printf("Epoch limit reached, returning corridors\n");
            return corridors;
        }

        // if we have run 100+ iterations, reset the matrix & corridors and try again
        if(iterationCount > 100) {
            // printf("Resetting matrix and corridors\n");
            placed = 0;
            iterationCount = 0;
            epoch++;
            for (int i = 0; i < numCorridors; i++) {
                struct Rectangle noCorridor = {-1, -1, -1, -1};
                corridors[i] = noCorridor;
            }
            for (int i = 0; i < ROWS; i++) {
                for (int j = 0; j < COLS; j++) {
                    matrix[i][j] = initialMatrix[i][j];
                }
            }
            // reset connections
            for (int i = 0; i < numRooms; i++) {
                for (int j = 0; j < numRooms; j++) {
                    connections[i][j] = 0;
                }
            }
        }
    }
    printf("corridors completed on epoch: %d\n", epoch);
    return corridors;
}


void printCorridors(char matrix[][COLS], struct Rectangle *corridors, int numCorridors) {
    printf("Corridors:\n");
    for (int i = 0; i < numCorridors; i++) {
        char corridorChar = matrix[corridors[i].yPos][corridors[i].xPos];
        printf("Corridor %d: x=%d, y=%d, width=%d, height=%d, char='%c'\n", i, corridors[i].xPos, corridors[i].yPos, corridors[i].width, corridors[i].height, corridorChar);
    }
}


int isLetter(char c) {
    if (c >= 'a' && c <= 'z') {
        return 1;
    }
    return 0;
}


struct Point *placeDoors(char matrix[ROWS][COLS], int maxDoors) {
    struct Point *doors = malloc(maxDoors * sizeof(struct Point));
    int placed = 0;
    for (int i = 1; i < ROWS - 1; i++) {
        for (int j = 1; j < COLS - 1; j++) {
            // Check if the cell is a number
            if (matrix[i][j] >= '0' && matrix[i][j] <= '9') {
                // Check the eight neighbors
                int left = isLetter(matrix[i-1][j]);
                int right = isLetter(matrix[i+1][j]);
                int up = isLetter(matrix[i][j-1]);
                int down = isLetter(matrix[i][j+1]);
                int topLeft = isLetter(matrix[i-1][j-1]);
                int topRight = isLetter(matrix[i-1][j+1]);
                int bottomLeft = isLetter(matrix[i+1][j-1]);
                int bottomRight = isLetter(matrix[i+1][j+1]);
                if (left + right + up + down + topLeft + topRight + bottomLeft + bottomRight == 2) {
                    // Precisely 2 neighbors are letters, so place a door
                    matrix[i][j] = '%';
                    doors[placed] = (struct Point) {j, i};
                    placed++;
                }
            }
        }
    }
    return doors;
}


//// TODO: use this function to detect when a player is in a room and to display it 
//         and its contents (monsters, exists, treasure, etc.)
int detectPlayerRoom(struct Rectangle *rooms, int ROOM_COUNT, struct Point playerLocation) {
    for (int i = 0; i < ROOM_COUNT; i++) {
        struct Rectangle room = rooms[i];
        // Check if the player is in the current room
        if (playerLocation.x >= room.xPos && playerLocation.x < room.xPos + room.width &&
            playerLocation.y >= room.yPos && playerLocation.y < room.yPos + room.height) {
            // Return the index of the room the player is in
            return i;
        }
    }
    // Return -1 if the player is not in any room
    return -1;
}


void fillRectWithStars(char matrix[][COLS], struct Rectangle *rects, int rectIndex, char fillChar) {
    struct Rectangle rect = rects[rectIndex];
    for (int x = rect.xPos + 1; x < rect.xPos + rect.width - 1; x++) {
        for (int y = rect.yPos + 1; y < rect.yPos + rect.height - 1; y++) {
            matrix[y][x] = fillChar;
        }
    }
}


struct Point randomPointInRectangle(struct Rectangle rect) {
    struct Point point;
    point.x = rect.xPos + 1 + rand() % (rect.width - 2);
    point.y = rect.yPos + 1 + rand() % (rect.height - 2);
    return point;
}


// returns an array of ints where each index i is the 
// number of connections room i has
int* countConnections(int numRooms, int connections[][numRooms]) {
    int* connectionsCount = malloc(numRooms * sizeof(int));
    for (int i = 0; i < numRooms; i++) {
        connectionsCount[i] = 0;
        for (int j = 0; j < numRooms; j++) {
            if (connections[i][j]) {
                connectionsCount[i]++;
            }
        }
    }
    return connectionsCount;
}


int findFirstNonIgnoredOne(int* arr, int length, int ignoreIndex1, int ignoreIndex2) {
    for (int i = 0; i < length; i++) {
        if (i == ignoreIndex1 || i == ignoreIndex2) {
            continue;
        }
        if (arr[i] == 1) {
            return i;
        }
    }
    return -1;
}


//// TODO: write function that detectsPlayerCorridor
//// TODO: write function that detects when a player has entered a room and floods that 
//         room w/ a char to indicate that it is now visible, and it also places anything 
//         that exists in the room, such as monsters, treasure, exits, etc.


int main()
{
    randSeed = 0;
    //// TODO: change back when in prod
    // srand(time(NULL));
    srand(randSeed);
    char matrix[ROWS][COLS]; // the board
    char input; // character move input: 'wasd' or 'q'
    // initialize display message that gives player info regarding out of bounds, etc.
    char message[80];
    strcpy(message, "");
    // initialize player 1 at 0,0
    // DONE: make playerLocation a Point struct
    // DONE: set player's location to be in a room (make sure to prevent placement on a wall)
    // DONE: set player's location to be in the "first room" which may be farther away 
    // from the level end, or simply just in room #0 (went with topleft room for now)
    struct Point playerLocation = {-1, -1};
    struct Point exitLocation = {-1, -1};
    struct Point treasureLocation = {-1, -1};
    // store the tile type the player is on currently
    char playerCell = '?';

    struct Rectangle *rooms;
    struct Rectangle topLeftRoom;
    int indexOfTopLeftRoom;
    int treasureRoomIndex;
    int connections[ROOM_COUNT][ROOM_COUNT]; // an adjacency matrix to store connections between rooms
    struct Rectangle *corridors; // the collection of hallways connecting the rooms
    int* connectionsCount; // an array of ints where each index i is the number of connections room i has
    int roomEpochs = 0;

    // setup loop to generate a level
    while(1) {
        // clear the board
        fillMatrix(matrix, ROWS, COLS, '-');
        // place rooms
        rooms = placeRooms(matrix);

        // find the top left room
        topLeftRoom = findTopLeftRoom(rooms, ROOM_COUNT);

        // get the topLeftRoom index
        indexOfTopLeftRoom = getRoomIndexFromRect(topLeftRoom, rooms, ROOM_COUNT);

        // print the details of the top left room (debugging message)
        // printRoomDetails(matrix, rooms, indexOfTopLeftRoom);

        // initialize connections to all 0's to indicate there are no room connections yet
        for (int i = 0; i < ROOM_COUNT; i++) {
            for (int j = 0; j < ROOM_COUNT; j++) {
                connections[i][j] = 0;
            }
        }

        // debug_once = 1;
        // place corridors
        corridors = placeCorridors(matrix, rooms, ROOM_COUNT, CORRIDOR_COUNT, connections);

        // print corridors (debugging message)
        // printCorridors(matrix, corridors, CORRIDOR_COUNT);

        // get the 8th corridor from corridors
        struct Rectangle eighthCorridor = corridors[7];
        if(eighthCorridor.xPos == -1) {
            printf("Error: eighth corridor not found, resetting rooms\n");
            printf("Ending room epoch %d\n", roomEpochs);
            if(roomEpochs > MAX_ROOM_EPOCHS) {
                printf("Room epoch limit reached, returning to main\n");
                printf("There was difficulty rendering the level, please try again\n");
                return 1;
            }
            // clear rooms and start over
            // for (int i = 0; i < ROOM_COUNT; i++) {
            //     struct Rectangle noRoom = {-1, -1, -1, -1};
            //     rooms[i] = noRoom;
            // }
        } else {
            printf("Eighth corridor found, continuing with level setup\n");
            break;
        }
    }

    // denote the farthest room from the player's initial starting room
    int farthestRoomIndex = farthestRoom(indexOfTopLeftRoom, ROOM_COUNT, connections);
    printf("The farthest room from the top left room is room %d\n", farthestRoomIndex);

    // redraw the rooms so that they appear "on top of" the corridors
    //// TODO: uncomment this line when not debugging
    redrawAllRooms(matrix, rooms, 9);

    // place doors
    placeDoors(matrix, MAX_DOORS);

    // mark top left room as "visible" by filling it with stars
    //// TODO: uncomment this line when not debugging
    fillRectWithStars(matrix, rooms, indexOfTopLeftRoom, '*');

    // DONE: player player 1 at the topleft of the top left room
    // initially place player onto the board
    //// TODO: place player 1 at a random spot in the top left room
    playerLocation.x = topLeftRoom.xPos+2;
    playerLocation.y = topLeftRoom.yPos+2;
    playerCell = matrix[playerLocation.y][playerLocation.x];
    matrix[playerLocation.y][playerLocation.x] = PLAYER_CHAR;

    //// DONE: place the exit 'E' in the farthest room, at a random point in the room
    // place exit onto the board for now
    exitLocation = randomPointInRectangle(rooms[farthestRoomIndex]);
    matrix[exitLocation.y][exitLocation.x] = EXIT_CHAR;

    // count the number of connections each room has
    connectionsCount = countConnections(ROOM_COUNT, connections);
    // debugging message
    // for (int i = 0; i < ROOM_COUNT; i++) {
    //     printf("Room %d has %d connections\n", i, connectionsCount[i]);
    // }

    treasureRoomIndex = findFirstNonIgnoredOne(connectionsCount, ROOM_COUNT, indexOfTopLeftRoom, farthestRoomIndex);
    printf("The treasure room is room %d\n", treasureRoomIndex);
    // place the treasure in the treasureRoom
    treasureLocation = randomPointInRectangle(rooms[treasureRoomIndex]);
    matrix[treasureLocation.y][treasureLocation.x] = TREASURE_CHAR;
    //// TODO: place treasure in a room that has only one connection that isn't the starting nor ending room

    // main game loop
    while (1)
    {
        // clear the console
        // system("clear"); // TODO: uncomment this line when not debugging
        // print message
        printf("%s\n", message);
        // print the board w/ player on it
        printMatrix(matrix, ROWS, COLS);
        // take in user input WASD to move player 1
        printf("Enter a direction to move (wasd) or q to quit: ");
        scanf(" %c", &input); // TIL: space before %c to skip whitespace, including newline
        if (input == 'q')
        {
            printf("Quitting...\n");
            break;
        }
        else // not quitting
        {
            if (pointIsDoorOrFloor(matrix, destinationPoint(playerLocation, input))) {            
                // Before updating the player's position, restore the previous cell
                matrix[playerLocation.y][playerLocation.x] = playerCell;
                // clear the message
                strcpy(message, "");
                // Update the player's position
                playerLocation = destinationPoint(playerLocation, input);
                // Store the original character of the new cell and place the player
                playerCell = matrix[playerLocation.y][playerLocation.x];
                matrix[playerLocation.y][playerLocation.x] = PLAYER_CHAR;
            } else if (matrix[destinationPoint(playerLocation, input).y][destinationPoint(playerLocation, input).x] == EXIT_CHAR) {
                strcpy(message, "You win!");
                matrix[playerLocation.y][playerLocation.x] = playerCell; // restore prev cell tile
                playerLocation = destinationPoint(playerLocation, input); // update player location
                matrix[playerLocation.y][playerLocation.x] = PLAYER_CHAR; // place player
                break;
            } else if (matrix[destinationPoint(playerLocation, input).y][destinationPoint(playerLocation, input).x] == TREASURE_CHAR) {
                strcpy(message, "You found the treasure!");
                // update treasure location to -1, -1
                // update swap tile ("player cell") under player to be a floor tile
                // update player location to be the treasure location
                treasureLocation = (struct Point) {-1, -1};
                matrix[playerLocation.y][playerLocation.x] = playerCell; // restore prev cell tile
                playerLocation = destinationPoint(playerLocation, input); // update player location
                playerCell = '.'; // store blank cell tile to update after player moves away from where the treasure was
                matrix[playerLocation.y][playerLocation.x] = PLAYER_CHAR; // place player
            } else {
                if(!pointIsDoorOrFloor(matrix, destinationPoint(playerLocation, input))) {
                    strcpy(message, "Invalid move");
                } else {
                    strcpy(message, "Unknown error, code 001");
                }
            }
        }
    }

    // finally, print the last message and board state before ending the game
    // clear the console
    // system("clear"); // TODO: uncomment this line when not debugging
    // print message
    printf("%s\n", message);
    // print the board w/ player on it
    printMatrix(matrix, ROWS, COLS);

    printf("Thanks for playing!\n");
    free(rooms);
    free(corridors);
    free(connectionsCount);

    return 0;
}