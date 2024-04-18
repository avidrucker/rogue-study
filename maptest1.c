#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

int moveInBounds(char matrix[][COLS], struct Point playerPos, char direction)
{
    if (direction == 'w' && playerPos.y > 0)
    {
        return 1;
    }
    else if (direction == 'a' && playerPos.x > 0)
    {
        return 1;
    }
    else if (direction == 's' && playerPos.y < ROWS - 1)
    {
        return 1;
    }
    else if (direction == 'd' && playerPos.x < COLS - 1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int rectCollision(struct Rectangle a, struct Rectangle b)
{
    if (a.xPos < b.xPos + b.width &&
        a.xPos + a.width > b.xPos &&
        a.yPos < b.yPos + b.height &&
        a.yPos + a.height > b.yPos)
    {
        return 1;
    }
    else
    {
        return 0;
    }
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

// TIL: You cannot return an array from a function in C, but you can return a pointer to an array
struct Rectangle *placeRooms(char matrix[][COLS], int numRooms)
{
    int i, x, y, width, height;

    int placed = 0;
    int count = 0;
    struct Rectangle *rooms = malloc(numRooms * sizeof(struct Rectangle));
    while (placed < numRooms)
    {
        width = rand() % 10 + 7;
        height = rand() % 10 + 7;
        x = rand() % (COLS - width - 2) + 2;  // min 2, max ROWS - height - 2
        y = rand() % (ROWS - height - 2) + 2; // min 2, max COLS - width - 2
        struct Rectangle c = {x, y, width, height};
        if (placed > 0)
        {
            int collisionFound = 0;
            for (i = 0; i < placed; i++)
            {
                struct Rectangle grown = {rooms[i].xPos - 1, rooms[i].yPos - 1, rooms[i].width + 2, rooms[i].height + 2};
                if (rectCollision(c, grown))
                {
                    collisionFound = 1;
                    break;
                }
            }

            if (collisionFound)
            {
                // printf("collision found\n");
            }
            else
            {
                // debugging
                // printf("placing room %d\n", (placed + 1));
                // printf("x: %d, y: %d, width: %d, height: %d\n", x, y, width, height);
                placeRoom(matrix, x + 1, y + 1, width - 2, height - 2, i + '0');
                rooms[placed] = c;
                placed++;
            }
        }
        else
        {
            // debugging
            // printf("placing first room\n");
            // printf("x: %d, y: %d, width: %d, height: %d\n", x, y, width, height);
            placeRoom(matrix, x + 1, y + 1, width - 2, height - 2, i + '0');
            rooms[placed] = c;
            placed++;
        }
        count++;
        if (count > 50)
        {
            // clear rooms & start over
            placed = 0;
            count = 0;
            for (int i = 0; i < numRooms; i++)
            {
                struct Rectangle noRoom = {-1, -1, -1, -1};
                rooms[i] = noRoom;
            }
            fillMatrix(matrix, ROWS, COLS, '-');
        }
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

    struct Rectangle topLeftRoom = rooms[0]; // Initialize with the first room
    int roomIndex = 0;
    for (int i = 1; i < numRooms; i++) {
        roomIndex++;
        if (rooms[i].xPos < topLeftRoom.xPos || rooms[i].yPos < topLeftRoom.yPos) {
            topLeftRoom = rooms[i];
        }
    }

    // debugging message
    printf("The top left room is room number %d is at (%d, %d)\n", roomIndex, topLeftRoom.xPos, topLeftRoom.yPos);

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


int pointInRect(struct Point point, struct Rectangle rect) {
    if (point.x >= rect.xPos && point.x < rect.xPos + rect.width &&
        point.y >= rect.yPos && point.y < rect.yPos + rect.height) {
        return 1;
    }
    return 0;
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

    printf("Initial matrix fully transitive: %d\n", isFullyTransitive(numRooms, connections));

    struct Rectangle *corridors = malloc(numCorridors * sizeof(struct Rectangle));
    while (isFullyTransitive(numRooms, connections) == 0 && placed < numCorridors) {
        int isTall = rand() % 2; // 0 for wide, 1 for tall
        int width = isTall ? 3 : rand() % 10 + 7;
        int height = isTall ? rand() % 10 + 7 : 3;
        int x = rand() % (COLS - width - 2) + 2;
        int y = rand() % (ROWS - height - 2) + 2;
        struct Rectangle corridor = {x, y, width, height};

        int intersectedRooms[3]; // changed to 3 to include the case where the corridor intersects more than 2 rooms
        int intersections = 0;
        for (int i = 0; i < numRooms; i++) {
            struct Rectangle shrunkRoom = {rooms[i].xPos+2, rooms[i].yPos+2, rooms[i].width-4, rooms[i].height-4};
            if (rectCollision(corridor, shrunkRoom)) {
                if(intersections > 2) {
                    printf("Error: corridor intersects more than 2 rooms\n");
                    break;
                } else {
                    intersectedRooms[intersections] = i;
                    intersections++;
                }
            }
        }

        if (intersections == 2) {
            // Check if the rooms are already connected
            if (connections[intersectedRooms[0]][intersectedRooms[1]]) {
                // The rooms are already connected, so skip this corridor
                printf("Rooms %d and %d are already connected, skipping adding this corridor\n", intersectedRooms[0], intersectedRooms[1]);
            } else {
                // Place the corridor
                printf("Placing corridor %c\n", placed + 'a');
                placeRoom(matrix, x, y, width, height, placed + 'a');
                corridors[placed] = corridor;
                placed++;
                // Update the connections matrix
                connections[intersectedRooms[0]][intersectedRooms[1]] = 1;
                connections[intersectedRooms[1]][intersectedRooms[0]] = 1;
            }
        }
        iterationCount++;

        if(epoch > 50) {
            // TODO: either throw an error here, increase the epoch limit, or figure out a different strategy
            printf("Epoch limit reached, returning corridors\n");
            return corridors;
        }

        // if we have run 100+ iterations, reset the matrix & corridors and try again
        if(iterationCount > 100) {
            printf("Resetting matrix and corridors\n");
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
}

int main()
{
    srand(time(NULL));
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
    struct Point playerLocation = {0, 0};
    // remember the tile type the player is on currently
    char playerCell = '-';
    // clear the board
    fillMatrix(matrix, ROWS, COLS, '-');
    // place rooms
    struct Rectangle *rooms = placeRooms(matrix, ROOM_COUNT);

    // find the top left room
    struct Rectangle topLeftRoom = findTopLeftRoom(rooms, ROOM_COUNT);

    // get the topLeftRoom index
    int indexOfTopLeftRoom = getRoomIndexFromRect(topLeftRoom, rooms, ROOM_COUNT);

    // print the details of the top left room (debugging message)
    printRoomDetails(matrix, rooms, indexOfTopLeftRoom);

    // an adjacency matrix to store connections between rooms
    int connections[ROOM_COUNT][ROOM_COUNT]; 
    // initialize connections to all 0's to indicate there are no room connections yet
    for (int i = 0; i < ROOM_COUNT; i++) {
        for (int j = 0; j < ROOM_COUNT; j++) {
            connections[i][j] = 0;
        }
    }

    // place corridors
    struct Rectangle *corridors = placeCorridors(matrix, rooms, ROOM_COUNT, 15, connections);

    // redraw the rooms so that they appear "on top of" the corridors
    redrawAllRooms(matrix, rooms, 9);

    // TODO: player player 1 at the topleft of the top left room
    // initially place player onto the board
    playerLocation.x = topLeftRoom.xPos+2;
    playerLocation.y = topLeftRoom.yPos+2;
    matrix[playerLocation.y][playerLocation.x] = 'P';

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
            if (moveInBounds(matrix, playerLocation, input)) {            
                // Before updating the player's position, restore the previous cell
                matrix[playerLocation.y][playerLocation.x] = playerCell;
                // clear the message
                strcpy(message, "");
                // Update the player's position
                if (input == 'w') {
                    playerLocation.y--;
                } else if (input == 'a') {
                    playerLocation.x--;
                } else if (input == 's') {
                    playerLocation.y++;
                } else if (input == 'd') {;
                    playerLocation.x++;
                } else {
                    strcpy(message, "Error code 1: Invalid input");
                }
                // Store the original character of the new cell and place the player
                playerCell = matrix[playerLocation.y][playerLocation.x];
                matrix[playerLocation.y][playerLocation.x] = 'P';
            } else {
                strcpy(message, "Invalid move");
            }
        }
    }
    printf("Thanks for playing!\n");
    free(rooms);

    return 0;
}