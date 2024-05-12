#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

/* DESCRIPTION OF STUDY

This version of the study focuses on generating non-rectangular, non-straight corridors
to connect the rooms randomly until all rooms are traversible.

*/

/* Notes:
- The player is represented by the character '@'
- The exit is represented by the character 'E'
- The treasure is represented by the character 'T'
- Currently, each room has an index from 0 to 8, and is represented internally by a number from 0 to 8
- As far as rendering goes, room wall are rendered currently with the character '0' to '8' to represent
  in which quandrant the room is located in, with 0 in the upper left corner and 8 in the lower right
*/

/* Ideas:
- A magic sensor that grows warmer or colder when the player is closer or farther from the treasure
- Fog of war that reveals more of the map as the player moves into a room or corridor
- Hidden/secret doors and room that are only revealed if the player moves into/enters them
*/

// DONE: make sure all rooms are traversable using the connections matrix
// DONE: place the treasure in the farthest room from the exit that is not the starting room
/// TODO: feat conversion of room-corridor intersections to doors
/// TODO: feat walking through '#' corridors
/// TODO: feat maximum of 1 door/corridor connection per wall
// DONE: figure out which room is the "first room" and place player there
// DONE: place the exit in the "last room" (the room farthest from the first room)
// DONE: add a win condition when the player reaches the exit
/// IDEA: find a room that isn't the first nor last room that is has the next least room
//       connections, and mark this as the secret room to put treasure in. If no
//       such room exists, then swap the last room with 2nd to last room and make
//       the last room now be the secret room. eg: rooms 2->7->0->8->4 and 0->3 also.
//       In this example, room 2 is the first room, room 3 is the secret room, and 
//       room 4 is the last room. If there are two rooms or more that are only 
//       connected to one other room, then randomly choose one of them to be the 
//       secret room.

struct Rectangle
{
    int xPos;
    int yPos;
    int width;
    int height;
    char wallChar;
};

struct Point {
    int x; // column, starting at the left w/ col 0
    int y; // row, starting at the top w/ row 0
};

int ROWS = 30;
int COLS = 60;
int MAX_ROOM_COUNT = 9;
char PLAYER_CHAR = '@';
char EXIT_CHAR = 'E';
char TREASURE_CHAR = 'T';
int randSeed = 0; // NULL means random
int printNotQuit = 1; // when we quit, we don't reprint the board (1 means print the board, 0 means don't print the board)
int quadrantsUsed[9] = {-1}; // To track used quadrants
int wallsUsed[9][4] = {{0}}; // To track used walls between rooms
// int neighbors[8] = {-1}; // To track the neighbors of a room
int neighborsSimple[8] = {-1}; // To track the neighbors of a room
struct Point* firstWallPoint; // To track the start of a corridor
struct Point* secondWallPoint; // To track the end of a corridor

void clearNeighbors(int neighbors[]) {
    for (int i = 0; i < 8; i++) {
        neighbors[i] = -1;
    }
}

void clearNeighborsSimple(int neighbors[]) {
    for (int i = 0; i < 8; i++) {
        neighbors[i] = -1;
    }
}

// debugging function
void printQuadrantsUsed (int quadsUsed[9]) {
    for (int i = 0; i < 9; i++) {
        printf("Quadrant %d: %d\n", i, quadsUsed[i]);
    }
}

////
// debugging function
void printWallsUsed (struct Rectangle *rooms, int numRooms) {
    for (int i = 0; i < numRooms; i++) {
        printf("Room %d at quad %d walls: %d, %d, %d, %d\n", i, rooms[i].wallChar-'0', wallsUsed[i][0], wallsUsed[i][1], wallsUsed[i][2], wallsUsed[i][3]);
    }
}


/**
 * Fills a matrix with a specified character.
 *
 * This function takes a 2D character matrix, the number of rows and columns in the matrix,
 * and a character input. It fills the entire matrix with the specified character.
 *
 * @param matrix The 2D character matrix to be filled.
 * @param rows The number of rows in the matrix.
 * @param cols The number of columns in the matrix.
 * @param input The character to fill the matrix with.
 */
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

void fillQuadrant(char matrix[][COLS], int quadIndex, char fillChar) {
    int thirdWidth = COLS / 3;
    int thirdHeight = ROWS / 3;
    int xStart = quadIndex % 3 * thirdWidth;
    int yStart = quadIndex / 3 * thirdHeight;
    for (int i = yStart; i < yStart + thirdHeight; i++) {
        for (int j = xStart; j < xStart + thirdWidth; j++) {
            matrix[i][j] = fillChar;
        }
    }
}

/**
 * Prints a matrix to the console.
 *
 * This function takes a 2D character matrix, the number of rows and columns in the matrix,
 * and prints the matrix to the console.
 *
 * @param matrix The 2D character matrix to be printed.
 * @param rows The number of rows in the matrix.
 * @param cols The number of columns in the matrix.
 */
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

/// TODO: rewrite to pointIsDoorFloorOrCorridor
/**
 * Checks if a point is a door or floor tile.
 *
 * This function takes a 2D character matrix and a point, and checks if the character at the
 * specified point is a door, floor, or corridor tile.
 *
 * @param matrix The 2D character matrix to check.
 * @param point The point to check.
 * @return 1 if the character at the point is a door, floor, or corridor tile, 0 otherwise.
 */
int pointIsDoorFloorOrCorridor(char matrix[][COLS], struct Point point) {
    if (matrix[point.y][point.x] == '%' || matrix[point.y][point.x] == '.' || 
            matrix[point.y][point.x] == '#') {
        return 1;
    }
    return 0;
}

/**
 * Returns the destination point based on the player's current position and direction.
 *
 * This function takes the player's current position and a direction, and returns the point
 * that the player would move to if they moved in that direction.
 *
 * @param playerPos The player's current position.
 * @param direction A char representing the direction the player wants to move in.
 * @return The point the player would move to if they moved in the specified direction.
 */
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

/**
 * Checks if a point is within a rectangle.
 *
 * This function takes a rectangle and a point, and checks if the point is within the rectangle.
 *
 * @param rect The rectangle to check.
 * @param point The point to check.
 * @return 1 if the point is within the rectangle, 0 otherwise.
 */
int pointInRect(struct Rectangle rect, struct Point point) {
    if (point.x >= rect.xPos && point.x < rect.xPos + rect.width &&
        point.y >= rect.yPos && point.y < rect.yPos + rect.height) {
        return 1;
    }
    return 0;
}


/**
 * Places a room in the given matrix at the specified position with the specified dimensions.
 *
 * @param matrix The matrix to place the room in.
 * @param x The x-coordinate of the top-left corner of the room.
 * @param y The y-coordinate of the top-left corner of the room.
 * @param width The width of the room.
 * @param height The height of the room.
 * @param wallChar The character used to represent the walls of the room.
 */
void placeRoom(char matrix[][COLS], int x, int y, int width, int height, char wallChar)
{
    int i, j;

    for (i = x; i < x + width; i++)
    {
        for (j = y; j < y + height; j++)
        {
            if (j == y || j == y + height - 1)
            {
                matrix[j][i] = '-';
            }
            else if (i == x || i == x + width - 1)
            {
                matrix[j][i] = '|';
            }
            else
            {
                matrix[j][i] = '.';
            }
        }
    }
}

int roomExists(int quadIndex) {
    // printf("Checking room at quad %d to see if it exists... ", quadIndex);
    if (quadrantsUsed[quadIndex] == -1) {
        // printf("Room at quadIndex %d does not exist\n", quadIndex);
        return 0;
    } else {
        // printf("\n");
        return 1;
    }
}

struct Rectangle *placeRooms(char matrix[][COLS]) {
    // srand(time(NULL)); // Seed the random number generator

    int numRooms = rand() % 5 + 5; // Random number of rooms between 5 and 9
    struct Rectangle *rooms = malloc(numRooms * sizeof(struct Rectangle));
    int placed = 0;
    int quadChar = '0';

    for (int i = 0; i < 9; i++) {
        quadrantsUsed[i] = -1; // reset used quadrants
    }

    // debugging
    // printf("Initial quandrants used:\n");
    // printQuadrantsUsed(quadrantsUsed);

    // for(int i = 0; i < 9; i++) {
    //     printf("%d\n", roomExists(i));
    // }

    while (placed < numRooms) {
        int quadrant = rand() % 9;
        while (quadrantsUsed[quadrant] != -1) { // Find an unused quadrant
            quadrant = (quadrant + 1) % 9;
        }

        int thirdWidth = COLS / 3;
        int thirdHeight = ROWS / 3;

        int xStart = quadrant % 3 * thirdWidth + 1; // Calculate quadrant position
        int yStart = quadrant / 3 * thirdHeight + 1;

        int maxWidth = thirdWidth - 2; // 2 tiles less than a third of the map's dimensions
        int maxHeight = thirdHeight - 2;

        // printf("maxWidth: %d, maxHeight: %d\n", maxWidth, maxHeight);

        int width = rand() % (maxWidth - 5 + 1) + 5; // Room size between 5x5 and maxWidth x maxHeight
        int height = rand() % (maxHeight - 5 + 1) + 5;

        // printf("width: %d, height: %d\n", width, height);

        // Ensure the room doesn't go out of its quadrant
        int x = xStart + rand() % (int)fmax((thirdWidth - width - 2), 1);
        int y = yStart + rand() % (int)fmax((thirdHeight - height - 2), 1);

        // printf("x: %d, y: %d\n", x, y);

        quadChar = '0' + quadrant;

        // printf("Placing room %d in quadrant %d at (%d, %d) with size %dx%d\n", placed, quadrant, x, y, width, height);
        struct Rectangle c = {x, y, width, height, quadChar};
        placeRoom(matrix, x, y, width, height, quadChar); // Place room on map
        rooms[placed] = c;
        quadrantsUsed[quadrant] = placed; // Mark this quadrant as used
        placed++;
    }
    
    // debugging
    // printf("Final quandrants used:\n");
    // printQuadrantsUsed(quadrantsUsed);

    // for(int i = 0; i < 9; i++) {
    //     printf("%d\n", roomExists(i));
    // }

    return rooms;
}

void redrawAllRooms(char matrix[][COLS], struct Rectangle *rooms, int numRooms) {
    for (int i = 0; i < numRooms; i++) {
        // placeRoom(matrix, rooms[i].xPos+1, rooms[i].yPos+1, rooms[i].width-2, rooms[i].height-2, i + '0');
        placeRoom(matrix, rooms[i].xPos, rooms[i].yPos, rooms[i].width, rooms[i].height, rooms[i].wallChar);
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

// Simplified adjacency for a 3x3 grid, index 0-8 from top-left to bottom-right
int adjacentSimple[9][9] = {
    {-1, 1, -1, 3, -1, -1, -1, -1, -1}, // quad 0 is adjacent to quads 1, 3
    {0, -1, 2, -1, 4, -1, -1, -1, -1}, // quad 1 is adjacent to quads 0, 2, and 4
    {-1, 1, -1, -1, -1, 5, -1, -1, -1}, // quad 2 is adjacent to quads 1 and 5
    {0, -1, -1, -1, 4, -1, 6, -1, -1}, // quad 3 is adjacent to quads 0, 4, and 6
    {-1, 1, -1, 3, -1, 5, -1, 7, -1}, // quad 4 is adjacent to quads 1, 3, 5, and 7
    {-1, -1, 2, -1, 4, -1, -1, -1, 8}, // quad 5 is adjacent to quads 2, 4, and 8
    {-1, -1, -1, 3, -1, -1, -1, 7, -1}, // quad 6 is adjacent to quads 3 and 7
    {-1, -1, -1, -1, 4, -1, 6, -1, 8}, // quad 7 is adjacent to quads 6, 4, and 8 
    {-1, -1, -1, -1, -1, 5, -1, 7, -1} // quad 8 is adjacent to quads 5 and 7
};

int areAdjacentSimple(int room1Index, int room2Index) {
    if(room1Index < 0 || room1Index > 8 || room2Index < 0 || room2Index > 8) {
        // printf("Error: rooms %d and %d are out of bounds\n", room1Index, room2Index);
        return -1;
    }
    return adjacentSimple[room1Index][room2Index];
}

void calculateNeighborsSimple(int roomIndex) {
    int upShift = roomIndex - 3;
    int rightShift = roomIndex + 1;
    int downShift = roomIndex + 3;
    int leftShift = roomIndex - 1;
    neighborsSimple[0] = areAdjacentSimple(roomIndex, upShift) != -1 ? upShift : -1;
    neighborsSimple[1] = areAdjacentSimple(roomIndex, rightShift) != -1 ? rightShift : -1;
    neighborsSimple[2] = areAdjacentSimple(roomIndex, downShift) != -1 ? downShift : -1;
    neighborsSimple[3] = areAdjacentSimple(roomIndex, leftShift) != -1 ? leftShift : -1;
}

char* directionsSimple[4] = {"North", "East", "South", "West"};

void printCurrentNeighborsSimple() {
    for (int i = 0; i < 4; i++) {
        if(neighborsSimple[i] != -1) {
            printf("Neighbor %d: quadrant %d to the %s\n", i, neighborsSimple[i], directionsSimple[i]);
        }
    }
}

int getFacingWallDirectionSimple(int room1Index, int room2Index) {
    int direction = areAdjacentSimple(room1Index, room2Index);
    if (direction == -1) {
        printf("Error: rooms %d and %d are not adjacent\n", room1Index, room2Index);
        return -1;
    }
    for(int i = 0; i < 8; i++) {
        if(neighborsSimple[i] == room2Index) {
            printf(">>> The wall of room %d that connects to room %d is on the %s\n", room1Index, room2Index, directionsSimple[i]);
            direction = i;
        }
    }
    return direction;
}

struct Point *getRandomPointOnWall(struct Rectangle room, int direction) {
    struct Point *point = (struct Point *)malloc(1 * sizeof(struct Point));
    
    switch (direction) {
        case 0: // North
            (*point).x = room.xPos + room.width/2;
            (*point).y = room.yPos - 1;
            break;
        case 1: // East
            (*point).x = room.xPos + room.width;
            (*point).y = room.yPos + room.height/2;
            break;
        case 2: // South
            (*point).x = room.xPos + room.width/2;
            (*point).y = room.yPos + room.height;
            break;
        case 3: // West
            (*point).x = room.xPos - 1;
            (*point).y = room.yPos + room.height/2;
            break;
        default:
            // Invalid direction
            printf("&&& Error: invalid direction %d\n", direction);
            (*point).x = -1;
            (*point).y = -1;
            break;
    }
    return point;
}


void printConnections(int numRooms, int connections[][numRooms]) {
    for (int i = 0; i < numRooms; i++) {
        for (int j = 0; j < numRooms; j++) {
            printf("%d ", connections[i][j]);
        }
        printf("\n");
    }
}


struct Rectangle *placeCorridors(char matrix[][COLS], struct Rectangle *rooms, int numRooms, int connections[][numRooms]) {
    // srand(time(NULL));

    /// TODO: assess whether numRooms is a good cap on corridors
    struct Rectangle *corridors = malloc(numRooms * sizeof(struct Rectangle));
    int placed = 0;
    char pathLetter = '#'; // 'a' or '1' for testing / '#'

    // debugging
    printf("=========\n");
    printf("before connecting the corridors the connections are:\n");
    printConnections(numRooms, connections);
    printf("=========\n");

    while (!isFullyTransitive(numRooms, connections)) {

        // pick two random rooms
        int room1Index = rand() % numRooms;
        int room2Index = (room1Index + 1 + rand() % (numRooms - 1)) % numRooms;
        int room1Quad = rooms[room1Index].wallChar - '0';
        int room2Quad = rooms[room2Index].wallChar - '0';

        /// TODO: rewrite this logic so that it is more readable
        // check to make sure that the rooms are in valid quadrants
        if(room1Quad < 0 || room1Quad > 8 || room2Quad < 0 || room2Quad > 8 || room1Quad == room2Quad) {
            // printf("Error: rooms %d or %d have invalid quads\n", room1Index, room2Index);
            // printf("Room index %d has a quad of %d\n", room1Index, room1Quad);
            // printf("Room index %d has a quad of %d\n", room2Index, room2Quad);
            continue;
        }


        /// TODO: see if we can remove this next check if we can determine it is redundant/unnecessary
        if(!roomExists(room1Quad) || !roomExists(room2Quad)) {
            // printf("Error: rooms %d or %d do not exist\n", room1Index, room2Index);
            // printf("Room index %d has a quad of %d\n", room1Index, room1Quad);
            // printf("Room index %d has a quad of %d\n", room2Index, room2Quad);
            continue;
        } else {
            // printf("Rooms %d and %d exist\n", room1Index, room2Index);
            // printf("Room index %d has a quad of %d\n", room1Index, room1Quad);
            // printf("Room index %d has a quad of %d\n", room2Index, room2Quad);
        }

        // room1 and room2 are not already connected and their quads are adjacet
        // we can place a corridor between them
        if (!connections[room1Index][room2Index] && areAdjacentSimple(room1Quad, room2Quad) != -1) {

            printf("Attempting to place a corridor between rooms %d and %d in quads %d and %d\n", room1Index, room2Index, room1Quad, room2Quad);   
            calculateNeighborsSimple(room1Quad);
            int wall1 = getFacingWallDirectionSimple(room1Quad, room2Quad);
            clearNeighborsSimple(neighborsSimple);

            calculateNeighborsSimple(room2Quad);
            int wall2 = getFacingWallDirectionSimple(room2Quad, room1Quad);
            printf("The wall directions are %d and %d to connect quads %d and %d\n", wall1, wall2, room1Quad, room2Quad);

            /// TODO: print out which walls are being used here

            if(wallsUsed[room1Index][wall1] || wallsUsed[room2Index][wall2]) {
                // printf("Error: wall %d of quad %d OR wall %d of quad %d are already used\n", wall1, room1Quad, wall2, room2Quad);
                continue;
            } else {
                firstWallPoint = getRandomPointOnWall(rooms[room1Index], wall1);
                secondWallPoint = getRandomPointOnWall(rooms[room2Index], wall2);
                wallsUsed[room1Index][wall1] = 1;
                wallsUsed[room2Index][wall2] = 1;
                // printf("A random point along the east wall of room %d is (%d, %d)\n", room1Quad, firstWallPoint->x, firstWallPoint->y);
                // printf("A random point along the west wall of room %d is (%d, %d)\n", room2Quad, secondWallPoint->x, secondWallPoint->y);
                clearNeighborsSimple(neighborsSimple);

                // pathLetter = 'a'; // for testing

                int x = firstWallPoint->x;
                int y = firstWallPoint->y;
                int target_x = secondWallPoint->x;
                int target_y = secondWallPoint->y;

                // if(pathLetter == '4') {
                //     printf("!!!\n");
                //     printf("Storing the startpoint of the corridor at (%d, %d)\n", x, y);
                // }
                printf("... marking from point (%d, %d) to point (%d, %d) for path number %c\n", x, y, target_x, target_y, pathLetter);
                printf("\n");

                int stepCounter = 0;
                // int totalSteps = abs(target_x - x) + abs(target_y - y);
                // Perform random walk from the 1st point to the 2nd point
                while (x != target_x || y != target_y) {
                    int moveInXDirection = (x != target_x) && ((y == target_y) || (rand() % 2));
                    int moveInYDirection = (y != target_y) && ((x == target_x) || (rand() % 2));

                    // || stepCounter >= totalSteps - 2
                    if (stepCounter < 2) {
                        // For the first two steps and last two steps, move in the direction away from the wall
                        if (wall1 == 0 || wall1 == 2) { // If the wall is on the north or south
                            moveInYDirection = 1;
                            moveInXDirection = 0;
                        } else { // If the wall is on the east or west
                            moveInXDirection = 1;
                            moveInYDirection = 0;
                        }
                    }

                    if (moveInXDirection) {
                        int step_x = (x > target_x) ? -1 : 1;
                        x += step_x;
                    }
                    else if (moveInYDirection) {
                        int step_y = (y > target_y) ? -1 : 1;
                        y += step_y;
                    }

                    matrix[y][x] = pathLetter; // Mark the corridor path
                    stepCounter++;
                }

                matrix[firstWallPoint->y][firstWallPoint->x] = '?'; // Mark the start of the corridor
                matrix[target_y][target_x] = '?'; // Mark the end of the corridor
                // if(pathLetter == '4') {
                //     printf("Storing the endpoint of the corridor at (%d, %d)\n", secondWallPoint->x, secondWallPoint->y);
                //     printf("!!!\n");
                // }

                corridors[placed].xPos = x; // Store endpoint for simplicity
                corridors[placed].yPos = y;
                connections[room1Index][room2Index] = 1;
                connections[room2Index][room1Index] = 1;
                placed++;

                // debugging
                // increment pathChar
                // pathLetter++;
            }
        }
    }
    // debugging
    // printWallsUsed(rooms, numRooms);

    // debugging
    // printf("=========\n");
    // printf("after connecting the corridors the connections are:\n");
    // printConnections(numRooms, connections);
    // printf("=========\n");

    return corridors;
}


int isLetter(char c) {
    if (c >= 'a' && c <= 'z') {
        return 1;
    }
    return 0;
}

int isPeriod(char c) {
    if (c == '.') {
        return 1;
    }
    return 0;
}

int isQuestionMark(char c) {
    if (c == '?') {
        return 1;
    }
    return 0;
}

int isWallOrPeriod(char c) {
    if (c == '|' || c == '-' || c == '.') {
        return 1;
    }
    return 0;
}

int isQorPeriod(char c) {
    if (c == '.' || c == '?') {
        return 1;
    }
    return 0;
}


/// TODO: optimize code to only look at indices where door indicators are (see output of placeCorridors)
/// TODO: rewrite logic so doors are placed when hallways/corridors are placed
// struct Point *
void placeDoors(char matrix[ROWS][COLS], int maxDoors) {
    // struct Point *doors = malloc(maxDoors * sizeof(struct Point));
    int placed = 0;
    for (int i = 1; i < ROWS - 1; i++) {
        for (int j = 1; j < COLS - 1; j++) {
            // Check if the cell is a wall
            if (matrix[i][j] == '-' || matrix[i][j] == '|') {
                // Check the eight neighbors
                int left = isQorPeriod(matrix[i-1][j]);
                int right = isQorPeriod(matrix[i+1][j]);
                int up = isQorPeriod(matrix[i][j-1]);
                int down = isQorPeriod(matrix[i][j+1]);
                int topLeft = isWallOrPeriod(matrix[i-1][j-1]);
                int topRight = isWallOrPeriod(matrix[i-1][j+1]);
                int bottomLeft = isWallOrPeriod(matrix[i+1][j-1]);
                int bottomRight = isWallOrPeriod(matrix[i+1][j+1]);
                if (left + right + up + down + topLeft + topRight + bottomLeft + bottomRight == 4) {
                    // Precisely 2 neighbors are letters, so place a door
                    matrix[i][j] = '%';
                    // doors[placed] = (struct Point) {j, i};
                    placed++;

                    // change corridor end to regular corridor tile
                    if(isQuestionMark(matrix[i-1][j])) // to the left 
                    {
                        matrix[i-1][j] = '#';
                    } 
                    else if (isQuestionMark(matrix[i+1][j])) // to the right
                    {
                        matrix[i+1][j] = '#';
                    } else if (isQuestionMark(matrix[i][j-1])) // above
                    {
                        matrix[i][j-1] = '#';
                    } else if (isQuestionMark(matrix[i][j+1])) // below
                    {
                        matrix[i][j+1] = '#';
                    }
                }
            }
        }
    }
    // return doors;
}


//// TODO: use this function to detect when a player is in a room and to display it 
//         and its contents (monsters, exists, treasure, etc.)
int detectPlayerRoom(struct Rectangle *rooms, int roomCount, struct Point playerLocation) {
    for (int i = 0; i < roomCount; i++) {
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


/**
 * Finds the first non-ignored element in the given array.
 *
 * @param arr The array to search.
 * @param length The length of the array.
 * @param ignoreIndex1 The index of the first element to ignore.
 * @param ignoreIndex2 The index of the second element to ignore.
 * @return The value of the first non-ignored element, or -1 if all elements are ignored.
 */
int findFirstNonIgnoredOne(int* arr, int length, int ignoreIndex1, int ignoreIndex2) {
    printf("Looking for a room that is neither index %d nor index %d...\n", ignoreIndex1, ignoreIndex2);
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

int findMinConnectedRoomOfNonIgnoredRooms(int* arr, int length, int ignoreIndex1, int ignoreIndex2) {
    printf("Looking for a room that is neither index %d nor index %d...\n", ignoreIndex1, ignoreIndex2);
    int minConnections = 1000;
    int minIndex = -1;
    for (int i = 0; i < length; i++) {
        if (i == ignoreIndex1 || i == ignoreIndex2) {
            continue;
        }

        if (arr[i] < minConnections) {
            minConnections = arr[i];
            minIndex = i;
        }
    }
    return minIndex;
}


//// TODO: write function that detectsPlayerCorridor
//// TODO: write function that detects when a player has entered a room and floods that 
//         room w/ a char to indicate that it is now visible, and it also places anything 
//         that exists in the room, such as monsters, treasure, exits, etc.


int countRooms(int quadsUsed[9]) {
    int count = 0;
    for (int i = 0; i < 9; i++) {
        if (quadsUsed[i] != -1) {
            count++;
        }
    }
    return count;
}


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
    int connections[MAX_ROOM_COUNT][MAX_ROOM_COUNT]; // an adjacency matrix to store connections between rooms
    struct Rectangle *corridors; // the collection of hallways connecting the rooms
    int* connectionsCount; // an array of ints where each index i is the number of connections room i has
    int roomEpochs = 0;
    int dynamicRoomCount = 0;

    // setup loop to generate a level
    while(1) {
        // clear the board
        fillMatrix(matrix, ROWS, COLS, ' ');

        // for testing, fill the board with symbols to make the quadrants visible
        // for (int i = 0; i < 9; i += 2) {
        //     fillQuadrant(matrix, i, '%');
        // }

        // place rooms
        rooms = placeRooms(matrix);

        dynamicRoomCount = countRooms(quadrantsUsed);

        printf("There are %d rooms\n", dynamicRoomCount);

        // find the top left room
        topLeftRoom = findTopLeftRoom(rooms, dynamicRoomCount);

        // get the topLeftRoom index
        indexOfTopLeftRoom = getRoomIndexFromRect(topLeftRoom, rooms, dynamicRoomCount);

        printf("The upper left most room is room %c at room index %d which is at roomQuadrant %d\n",
         rooms[indexOfTopLeftRoom].wallChar, indexOfTopLeftRoom, rooms[indexOfTopLeftRoom].wallChar - '0');

        // print the details of the top left room (debugging message)
        // printRoomDetails(matrix, rooms, indexOfTopLeftRoom);

        // initialize connections to all 0's to indicate there are no room connections yet
        for (int i = 0; i < MAX_ROOM_COUNT; i++) {
            for (int j = 0; j < MAX_ROOM_COUNT; j++) {
                connections[i][j] = 0;
            }
        }

        // place corridors
        corridors = placeCorridors(matrix, rooms, dynamicRoomCount, connections);

        // printf("Ending room and corridor generation...\n");
        break;
    }

    // denote the farthest room from the player's initial starting room
    int farthestRoomIndex = farthestRoom(indexOfTopLeftRoom, dynamicRoomCount, connections);
    // printf("The farthest room from the top left room is room %c at room index %d which is at roomQuadrant %d\n",
    //  rooms[farthestRoomIndex].wallChar, farthestRoomIndex, rooms[farthestRoomIndex].wallChar - '0');

    // redraw the rooms so that they appear "on top of" the corridors
    //// TODO: uncomment this line when not debugging
    // redrawAllRooms(matrix, rooms, dynamicRoomCount);

    /// TODO: reimplement doors after completing bendy corridors
    // place doors
    placeDoors(matrix, MAX_ROOM_COUNT * 2);

    // mark top left room as "visible" by filling it with stars
    //// TODO: uncomment this line when not debugging
    // fillRectWithStars(matrix, rooms, indexOfTopLeftRoom, '*');

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
    connectionsCount = countConnections(dynamicRoomCount, connections);
    // debugging message
    // for (int i = 0; i < dynamicRoomCount; i++) {
    //     printf("--- Room %d at quad %d has %d connections\n", i, rooms[i].wallChar - '0', connectionsCount[i]);
    // }

    /// TODO: place treasure in the room farthest from the exit, unless it is the starting room,
    ///       in which case then place the treasure room randomly in a room that isn't the starting room
    ///       nor the exit room
    int farthestFromExit = farthestRoom(farthestRoomIndex, dynamicRoomCount, connections);

    if(farthestFromExit == indexOfTopLeftRoom) {
        // printf("??? The farthest room from the exit is the starting room, placing treasure in a different room\n");
        treasureRoomIndex = findMinConnectedRoomOfNonIgnoredRooms(connectionsCount, dynamicRoomCount, indexOfTopLeftRoom, farthestRoomIndex);
        // printf("??? The treasure room index is %d\n", treasureRoomIndex);
    } else {
        // printf("--- The farthest room from the exit is room %c\n", rooms[farthestFromExit].wallChar);
        treasureRoomIndex = farthestFromExit;
    }

    // treasureRoomIndex = findFirstNonIgnoredOne(connectionsCount, dynamicRoomCount, indexOfTopLeftRoom, farthestRoomIndex);
    // printf("The treasure room is room %c\n", rooms[treasureRoomIndex].wallChar);
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
            printNotQuit = 0;
            break;
        }
        else // not quitting
        {
            if (pointIsDoorFloorOrCorridor(matrix, destinationPoint(playerLocation, input))) {            
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
                if(!pointIsDoorFloorOrCorridor(matrix, destinationPoint(playerLocation, input))) {
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
    if(printNotQuit) {
        printf("%s\n", message);
        // print the board w/ player on it
        printMatrix(matrix, ROWS, COLS);
    }

    printf("Thanks for playing!\n");
    free(rooms);
    free(corridors);
    free(connectionsCount);
    free(firstWallPoint);
    free(secondWallPoint);

    return 0;
}