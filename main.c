/******************************************************************************
 * File: main.c
 * Author: Vinh Nguyen
 * Date: April 03, 2026
 * 
 * * Description:
 * Main entry point and game loop state management for the interactive
 * Anteater Chess program. Orchestrates initialization, rendering, and 
 * the main execution loop for the game engine. Implements persistent I/O
 * game logging.
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "GameData.h"
#include "MoveValidation.h"

//=============================================================================

#define SZCODEVERSION "1.0.0"
#define MAX_INPUT_LENGTH 32

//=============================================================================

/*
 * Static helper to handle file I/O for game logging. 
 * Must be invoked PRIOR to ApplyMove() to accurately deduce capture states.
 */
static void LogMove(FILE* logFile, Board* pBoard, int turnCount, char color, int fRow, int fCol, int tRow, int tCol)
{
    if (!logFile) {
        return;
    }

    Piece mover = pBoard->grid[fRow][fCol];
    Piece target = pBoard->grid[tRow][tCol];

    char fFileStr = (char)(fCol + 'A');
    int fRankStr = fRow + 1;
    char tFileStr = (char)(tCol + 'A');
    int tRankStr = tRow + 1;

    const char* colorStr = (color == 'w') ? "White" : "Black";
    char flags[64] = "";

    /* Evaluate state prior to ApplyMove mutation to deduce optional flags */
    if (mover.type == 'K' && abs(tCol - fCol) == 2) {
        strcpy(flags, " (Castling)");
    }
    else if (mover.type == 'A' && target.type == 'P' && target.color != color) {
        strcpy(flags, " (Anteater Capture)");
    }
    else if (mover.type == 'P' && IsEnPassant(pBoard, fRow, fCol, tRow, tCol, color)) {
        strcpy(flags, " (En Passant)");
    }
    else if (mover.type == 'P' && ((color == 'w' && tRow == ROWS - 1) || (color == 'b' && tRow == 0))) {
        strcpy(flags, " (Promotion)");
    }

    fprintf(logFile, "%d. %s: %c%d -> %c%d%s\n", turnCount, colorStr, fFileStr, fRankStr, tFileStr, tRankStr, flags);
    
    /* Flush buffer immediately to prevent data loss on SIGINT or hard crash */
    fflush(logFile);
}

int main()
{
    /* Initializing the game board */
    Board gameBoard;
    InitializeBoard(&gameBoard);

    /* Prompts user for mode selection */
    char playerColor = ' ', gameMode = ' ', aiDifficulty = ' ';
    printf("Welcome to Anteater Chess by Team Hikaru Naka-more!\n");

    while (gameMode != '1' && gameMode != '2') {
        printf("\nPlease Select Mode\n1. Play a Friend\n2. Play Bots\n");

        /* Handle invalid input and clear stdin buffer */
        if (scanf(" %c", &gameMode) != 1) {
            while (getchar() != '\n');
        }
    }

    /* Skip side selection if PvP */
    if (gameMode == '1') {
        playerColor = 'w';
    }
    /* Prompts user for ai difficulty (to be used later) */
    else if (gameMode == '2') {
        while (aiDifficulty != '1' && aiDifficulty != '2' && aiDifficulty != '3') {
            printf("\nPlease Select Difficulty\n1. Easy\n2. Medium\n3. Hard\n");

            if (scanf(" %c", &aiDifficulty) != 1) {
                while (getchar() != '\n');
            }
        }
    }

    /* Prompt the user for side selection */
    while (playerColor != 'w' && playerColor != 'b') {    
        printf("\nPlease choose your side ('w' for white / 'b' for black): ");
        
        if (scanf(" %c", &playerColor) != 1) {
            while (getchar() != '\n');
        }
    }

    /* Clean up dangling \n after successful scanf */
    while (getchar() != '\n');

    /* Initialize file I/O for game logging */
    FILE* logFile = fopen("game_log.txt", "w");
    if (!logFile) {
        printf("CRITICAL WARNING: Unable to open game_log.txt for writing.\n");
    }

    /* Setup game variables */
    char moveInput[MAX_INPUT_LENGTH];
    char currentTurn = 'w';
    char fromCol, toCol;
    int fromRow, toRow, gameOver = 0;
    int fullTurnCount = 1;

    /* Begin game */
    while (!gameOver) {
        PrintBoard(&gameBoard);

        /* Pre-turn State Evaluation */
        if (IsInCheck(&gameBoard, currentTurn)) {
            if (IsCheckmate(&gameBoard, currentTurn)) {
                printf("\nCHECKMATE! %s wins the game.\n", (currentTurn == 'w') ? "Black" : "White");
                gameOver = 1;
                break;
            }
            printf("\nWARNING: %s is in CHECK!\n", (currentTurn == 'w') ? "White" : "Black");
        }

        int humanTurn = (gameMode == '1' || currentTurn == playerColor);
        if (humanTurn) {
            printf("\n%s's turn. Enter your move (e.g., 'E2 E4'): ",  (currentTurn == 'w') ? "White" : "Black");
            
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
                            
                        if (IsValidMove(&gameBoard, fRow, fCol, tRow, tCol, currentTurn)) {
                            
                            /* Call LogMove before applying state mutations to deduce capture data */
                            LogMove(logFile, &gameBoard, fullTurnCount, currentTurn, fRow, fCol, tRow, tCol);
                            
                            ApplyMove(&gameBoard, fRow, fCol, tRow, tCol);
                            
                            /* Standard chess turns increment after Black completes their move */
                            if (currentTurn == 'b') {
                                fullTurnCount++;
                            }
                            currentTurn = (currentTurn == 'w') ? 'b' : 'w';
                        }
                        else {
                            printf("Illegal move. Try again.\n");
                        }
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
                printf("\nInput stream closed. Exiting game.\n");
                gameOver = 1;
            }
        }
        else {
            /* * AI Integration Point: 
             * Must populate fRow, fCol, tRow, tCol using MoveList.c logic 
             * before calling LogMove() and ApplyMove()
             */
            printf("Bot Thinking... \n");
            
            /* Temporary break to prevent infinite loop while AI is unwritten */
            printf("AI module not implemented. Exiting.\n");
            gameOver = 1; 
        }
    }

    if (logFile) {
        fclose(logFile);
    }

    return 0;
}