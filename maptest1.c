#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct Rectangle {
    int xPos;
    int yPos;
    int width;
    int height;
};

int ROWS = 30;
int COLS = 60;

void fillMatrix(char matrix[][COLS], int rows, int cols, char input) {
    int i, j;

    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            matrix[i][j] = input;
        }
    }
}

void printMatrix(char matrix[][COLS], int rows, int cols) {
    int i, j;

    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            printf("%c ", matrix[i][j]);
        }
        printf("\n");
    }
}

int moveInBounds(char matrix[][COLS], int playerPos[], char direction) {
    if (direction == 'w' && playerPos[0] > 0) {
        return 1;
    } else if (direction == 'a' && playerPos[1] > 0) {
        return 1;
    } else if (direction == 's' && playerPos[0] < ROWS - 1) {
        return 1;
    } else if (direction == 'd' && playerPos[1] < COLS - 1) {
        return 1;
    } else {
        return 0;
    }
}

int rectCollision(struct Rectangle a, struct Rectangle b) {
    if (a.xPos < b.xPos + b.width &&
        a.xPos + a.width > b.xPos &&
        a.yPos < b.yPos + b.height &&
        a.yPos + a.height > b.yPos) {
        return 1;
    } else {
        return 0;
    }
}

void placeRoom(char matrix[][COLS], int x, int y, int width, int height, char wallChar) {
    int i, j;

    for (i = x; i < x + width; i++) {
        for (j = y; j < y + height; j++) {
            if (i == x || i == x + width - 1 || j == y || j == y + height - 1) {
                matrix[i][j] = wallChar;
            } else {
                matrix[i][j] = '.';
            }
        }
    }
}

void placeRooms(char matrix[][COLS], int numRooms) {
    int i, x, y, width, height;

    int placed = 0;
    struct Rectangle rooms[9] = {};
    while (placed < numRooms) {
        width = rand() % 9 + 7;
        height = rand() % 10 + 7;
        x = rand() % (ROWS - width - 2) + 2; // min 2, max ROWS - width - 2 
        y = rand() % (COLS - height - 2) + 2; // min 2, max COLS - height - 2
        // printf("x: %d, y: %d, width: %d, height: %d\n", x, y, width, height);
        struct Rectangle c = {x, y, width, height};
        if(placed > 0) {
            int collisionFound = 0;
            for (i = 0; i < placed; i++) {
                if (rectCollision(c, rooms[i])) {
                    collisionFound = 1;
                    break;
                }
            }
            
            if(collisionFound) {
                // printf("collision found\n");
            } else {
                printf("placing room %d\n", (placed + 1));
                placeRoom(matrix, x+1, y+1, width-2, height-2, i + '0');
                rooms[placed] = c;
                placed++;
            }
        } else {
            printf("placing first room\n");
            placeRoom(matrix, x+1, y+1, width-2, height-2, i + '0');
            rooms[placed] = c;
            placed++;
        }
    }
}

int main() {
    srand(time(NULL));
    char matrix[ROWS][COLS];
    char input;
    char message[80];
    // initialize player 1 at 0,0
    int playerLocation[2] = {0, 0};
    while(1) {
        // clear the console
        system("clear");
        // clear the board
        fillMatrix(matrix, ROWS, COLS, '-');
        // update player location on the board
        matrix[playerLocation[0]][playerLocation[1]] = 'P';
        // place rooms
        placeRooms(matrix, 9);
        // print message
        printf("%s\n", message);
        // print the board w/ player on it
        printMatrix(matrix, ROWS, COLS);
        // take in user input WASD to move player 1
        printf("Enter a direction to move (wasd) or q to quit: ");
        scanf(" %c", &input); // TIL: space before %c to skip whitespace, including newline
        if(input == 'q') {
            // strcpy(message, "Quitting...");
            printf("Quitting...\n");
            break;
        } else if (input == 'w') {
            if(moveInBounds(matrix, playerLocation, input)) {
                strcpy(message, "");
                playerLocation[0]--;
            } else {
                strcpy(message, "Invalid move");
            }
        } else if (input == 'a') {
            if(moveInBounds(matrix, playerLocation, input)) {
                strcpy(message, "");
                playerLocation[1]--;
            } else {
                strcpy(message, "Invalid move");
            }
        } else if (input == 's') {
            if(moveInBounds(matrix, playerLocation, input)) {
                strcpy(message, "");
                playerLocation[0]++;
            } else {
                strcpy(message, "Invalid move");
            }
        } else if (input == 'd') {
            if(moveInBounds(matrix, playerLocation, input)) {
                strcpy(message, "");
                playerLocation[1]++;
            } else {
                strcpy(message, "Invalid move");
            }
        }
    }
    printf("Thanks for playing!\n");

    return 0;
}