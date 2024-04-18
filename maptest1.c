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

int ROWS = 30;
int COLS = 60;

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

int moveInBounds(char matrix[][COLS], int playerPos[], char direction)
{
    if (direction == 'w' && playerPos[0] > 0)
    {
        return 1;
    }
    else if (direction == 'a' && playerPos[1] > 0)
    {
        return 1;
    }
    else if (direction == 's' && playerPos[0] < ROWS - 1)
    {
        return 1;
    }
    else if (direction == 'd' && playerPos[1] < COLS - 1)
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
                if (rectCollision(c, rooms[i]))
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
                printf("placing room %d\n", (placed + 1));
                printf("x: %d, y: %d, width: %d, height: %d\n", x, y, width, height);
                placeRoom(matrix, x + 1, y + 1, width - 2, height - 2, i + '0');
                rooms[placed] = c;
                placed++;
            }
        }
        else
        {
            printf("placing first room\n");
            printf("x: %d, y: %d, width: %d, height: %d\n", x, y, width, height);
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

int main()
{
    srand(time(NULL));
    char matrix[ROWS][COLS]; // the board
    char input; // character move input: 'wasd' or 'q'
    // initialize display message that gives player info regarding out of bounds, etc.
    char message[80];
    strcpy(message, "");
    // initialize player 1 at 0,0
    // TODO: make playerLocation a Point struct
    // TODO: set player's location to be in a room (make sure to prevent wall collisions)
    // TODO: set player's location to be in the "first room" which may be farther away 
    // from the level end, or simply just in room #0
    int playerLocation[2] = {0, 0};
    // remember the tile type the player is on currently
    char playerCell = '-';
    // clear the board
    fillMatrix(matrix, ROWS, COLS, '-');
    // place rooms
    struct Rectangle *rooms = placeRooms(matrix, 9);

    // initially place player onto the board
    matrix[playerLocation[0]][playerLocation[1]] = 'P';

    while (1)
    {
        // clear the console
        system("clear");
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
                matrix[playerLocation[0]][playerLocation[1]] = playerCell;
                // clear the message
                strcpy(message, "");
                // Update the player's position
                if (input == 'w') {
                    playerLocation[0]--;
                } else if (input == 'a') {
                    playerLocation[1]--;
                } else if (input == 's') {
                    playerLocation[0]++;
                } else if (input == 'd') {;
                    playerLocation[1]++;
                } else {
                    strcpy(message, "Error code 1: Invalid input");
                }
                // Store the original character of the new cell and place the player
                playerCell = matrix[playerLocation[0]][playerLocation[1]];
                matrix[playerLocation[0]][playerLocation[1]] = 'P';
            } else {
                strcpy(message, "Invalid move");
            }
        }
    }
    printf("Thanks for playing!\n");
    free(rooms);

    return 0;
}