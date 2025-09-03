#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#define COMPUTER 1
#define HUMAN 2
#define SIDE 3
#define COMPUTERMOVE 'O'
#define HUMANMOVE 'X'

int seed = 42;

// Show the help description
void showHelp(char *str) {
	printf("%s is a paper-and-pencil game for two players who take turns marking the spaces in a three-by-three grid with X or O. The player who succeeds in placing three of their marks in a horizontal, vertical, or diagonal row first is the winner.\n", str);
}

// Show command usage
void showUsage(char *str) {
	printf("Usage: %s [OPTIONS]\n", str);
	printf("Options:\n");
	printf("\t--help : print game description\n");
	printf("\t--seed INT : change the seed for pseudo-random number generation\n");
}

// Show the instructions
void showInstructions() {
	printf("\nTic-Tac-Toe\n\n");
	printf("----------------------------------------\n\n");
	printf("Choose a cell numbered from 0 to 8 as below and play!\n\n");
	printf(" 0 | 1 | 2 \n");
	printf("-----------\n");
	printf(" 3 | 4 | 5 \n");
	printf("-----------\n");
	printf(" 6 | 7 | 8 \n\n");
	printf("----------------------------------------\n\n");
}

// Initialize the game
void initialize(char board[][SIDE]) {
	for(int i=0; i < SIDE; i++)
		for(int j=0; j < SIDE; j++)
			board[i][j] = ' ';
}

// Display the game board
void showBoard(char board[][SIDE]) {
	printf(" %c | %c | %c \n", board[0][0], board[0][1], board[0][2]);
	printf("-----------\n");
	printf(" %c | %c | %c \n", board[1][0], board[1][1], board[1][2]);
	printf("-----------\n");
	printf(" %c | %c | %c \n\n", board[2][0], board[2][1], board[2][2]);
}

// Declare the winner of the game
void declareWinner(int whoseTurn) {
	if(whoseTurn == COMPUTER)
		printf("COMPUTER has won!\n");
	else
		printf("HUMAN has won!\n");
}

// Check if any row is crossed
int rowCrossed(char board[][SIDE]) {
	for(int i=0; i < SIDE; i++)
		if(board[i][0] == board[i][1] && board[i][1] == board[i][2] && board[i][0] != ' ')
			return 1;
	return 0;
}

// Check if any column is crossed
int columnCrossed(char board[][SIDE]) {
	for(int i=0; i < SIDE; i++)
		if(board[0][i] == board[1][i] && board[1][i] == board[2][i] && board[0][i] != ' ')
			return 1;
	return 0;
}

// Check if any diagonal is crossed
int diagonalCrossed(char board[][SIDE]) {
	if(board[0][0] == board[1][1] && board[1][1] == board[2][2] && board[0][0] != ' ')
		return 1;
	if(board[0][2] == board[1][1] && board[1][1] == board[2][0] && board[0][2] != ' ')
        	return 1;
	return 0;
}

// Function to check if the game is over
int gameOver(char board[][SIDE]) {
	return (rowCrossed(board) || columnCrossed(board) || diagonalCrossed(board));
}

// Super smart AI
int getAIMove(char board[][SIDE]) {
	static int moves[SIDE*SIDE] = {-1};

	if(moves[0] == -1) {
		for(int i=0; i < SIDE*SIDE; i++)
			moves[i] = i;
		for(int i=0; i < SIDE*SIDE; i++) {
			int j = rand()%(SIDE*SIDE), k = rand()%(SIDE*SIDE);
			int tmp = moves[j];
			moves[j] = moves[k];
			moves[k] = tmp;
		}
	}

	for(int i=0, k=0; i < SIDE; i++)
		for(int j=0; j < SIDE; j++, k++)
			if(board[i][j] == ' ') {
				board[i][j] = COMPUTERMOVE;
				int win = gameOver(board);
				board[i][j] = ' ';
				if(win)
					return k;
			}

	for(int i=0; i < SIDE*SIDE; i++) {
		int j=moves[i];
		if(board[j/SIDE][j%SIDE] == ' ')
			return j;
	}

	return -1;
}

// Play Tic-Tac-Toe
void playTicTacToe(int whoseTurn) {
	srand(seed);

	// 3x3 board
	char board[SIDE][SIDE];

	initialize(board);
	showInstructions();

	// Keep playing until the game is over or it is a draw
	int i;
	for(i=0; i < SIDE*SIDE; i++) {
		if(whoseTurn == COMPUTER) {
			//sleep(1);
			int move = getAIMove(board);
			assert(move >= 0);
			board[move/SIDE][move%SIDE] = COMPUTERMOVE;
			printf("COMPUTER has put a %c in cell %d %d\n", COMPUTERMOVE, move/SIDE, move%SIDE);
			whoseTurn = HUMAN;
		}
		else {
			for(;;) {
				int move;
				printf("Enter your move (0-8): ");
				scanf("%d", &move);
				printf("\n");
				if (move < 0 || move > 8) {
					printf("Invalid input! Please enter a number between 0 and 8.\n");
					continue;
				}
				else if(board[move/SIDE][move%SIDE] != ' ') {
					printf("Cell %d is already occupied. Try again.\n", move);
					continue;
				}
				else {
					board[move/SIDE][move%SIDE] = HUMANMOVE;
					break;
				}
			}
			whoseTurn = COMPUTER;
		}

		showBoard(board);
		printf("----------------------------------------\n\n");

		if(gameOver(board))
			break;
	}

	// If the game has drawn
	if(!gameOver(board))
		printf("It's a draw\n");
	else {
		// Undo last player change
		if(whoseTurn == COMPUTER)
			whoseTurn = HUMAN;
		else
			whoseTurn = COMPUTER;
		declareWinner(whoseTurn);
	}
}

// Driver program
int main(int argc, char **argv) {
	if(argc == 1) {
		playTicTacToe(COMPUTER);
	}
	else if(argc == 2) {
		if(strcmp(argv[1], "--help") == 0)
			showHelp(argv[0]);
		else
			showUsage(argv[0]);
	}
	else if(argc == 3) {
		if(strcmp(argv[1], "--seed") == 0) {
			int flag = 1;
			for(char *p=argv[2]; *p != '\0'; p++)
				flag &= isdigit(*p) != 0;
			if(flag) {
				seed = atoi(argv[2]);
				playTicTacToe(COMPUTER);
			}
			else
				showUsage(argv[0]);
		}
		else
			showUsage(argv[0]);
	}

	return 0;
}
