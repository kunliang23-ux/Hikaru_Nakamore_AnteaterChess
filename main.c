/******************************************************************************
 * File: main.c
 * Author: Vinh Nguyen
 * Date: April 03, 2026
 * 
 * * Description:
 * Main entry point and game loop state management for the interactive
 * Anteater Chess program. Orchestrates initialization, rendering, and 
 * the main execution loop for the game engine.
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "GameData.h"

//=============================================================================

#define SZCODEVERSION "1.0.0"
#define MAX_INPUT_LENGTH 32

//=============================================================================

int main()
{
    /* Initializing the game board */
    Board gameBoard;
    InitializeBoard(&gameBoard);

    /* Prompt the user for side selection */
    char playerColor = ' ';
    printf("Welcome to Anteater Chess by Team Hikaru Naka-more!\n");
    
    while (playerColor != 'w' && playerColor != 'b') {    
        printf("Please choose your side ('w' for white / 'b' for black): ");
        
        /* Handle invalid input and clear stdin buffer */
        if (scanf(" %c", &playerColor) != 1) {
            while (getchar() != '\n');
        }
    }

    /* Clean up dangling \n after successful scanf */
    while (getchar() != '\n');

    /* Setup game variables */
    char moveInput[MAX_INPUT_LENGTH];
    char aiColor = (playerColor == 'w') ? 'b' : 'w', currentTurn = 'w';
    char fromCol, toCol;
    int fromRow, toRow, gameOver = 0;

    /* Begin game */
    while (!gameOver) {
        PrintBoard(&gameBoard);

        if (currentTurn == playerColor) {
            printf("\nEnter your move (e.g., 'E2 E4'): ");
        
            if (fgets(moveInput, sizeof(moveInput), stdin) != NULL) {
                if (sscanf(moveInput, " %c%d %c%d", &fromCol, &fromRow, &toCol, &toRow) == 4) {
                    /* ASCII conversion */
                    int fCol = fromCol - (fromCol >= 'a' ? 'a' : 'A');
                    int fRow = fromRow - 1;
                    int tCol = toCol - (toCol >= 'a' ? 'a' : 'A');
                    int tRow = toRow - 1;

                    /* Validate bounds before accessing board array */
                    if ((fRow >= 0 && fRow < ROWS) && (tRow >= 0 && tRow < ROWS) && 
                        (fCol >= 0 && fCol < COLS) && (tCol >= 0 && tCol < COLS)) {
                        
                        MovePiece(&gameBoard, fRow, fCol, tRow, tCol);
                        currentTurn = aiColor;
                    }
                    else {
                        printf("Error: Coordinates out of bounds.\n");
                    }
                }
                else {
                    printf("Invalid input. Please try again.\n");
                }
            }
            else {
                /* Handle EOF (e.g., Ctrl+D on Linux servers) gracefully */
                printf("\nInput stream closed. Exiting game.\n");
                gameOver = 1;
            }
        }
        else {
            /* Placeholder for AI logic */
            currentTurn = playerColor;
        }
    }

    return 0;
}