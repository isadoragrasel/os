// C program to implement 2048 game

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#define SIDE 4

int seed = 42;
int arr[SIDE][SIDE] = {0}, prev[SIDE][SIDE], c[SIDE];

// Show the help description
void showHelp(char *str) {
	printf("%s is a single-player sliding tile puzzle video game. The objective of the game is to slide numbered tiles on a grid to combine them to create a tile with the number 2048.\n", str);
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
	printf("\n2048\n\n");
	printf("----------------------------------------\n\n");
	printf("Move numbers using W, S, A, and D and play!\n\n");
	printf("----------------------------------------\n\n");
}

// Function to print the game board
void showBoard() {
    for(int i=0; i < SIDE; i++) {
		for(int j=0; j < SIDE*7+1; j++)
			printf("-");
		printf("\n|");
        for(int j=0; j < SIDE; j++) {
            if(arr[i][j] != 0)
				printf(" %4d |", arr[i][j]);
			else
				printf("      |");
        }
		printf("\n");
    }
	for(int j=0; j < SIDE*7+1; j++)
		printf("-");
	printf("\n\n");
}

// Add a random number to the game board
void addNumber() {
    int i, j; // RANDOM INDEX
    do {
        i = rand()%SIDE;
        j = rand()%SIDE;
    } while(arr[i][j] != 0);

    int no = rand()%30;
    if(no == 0)
        arr[i][j] = 4;
    else if(no < 6)
        arr[i][j] = 2;
    else
		arr[i][j] = 1;
}

// Update the array after move
void update() {
    for(int i = SIDE-1; i > 0; i--) {
        if(c[i] == c[i-1]) {
            c[i] += c[i-1];
            c[i-1] = 0;
        }
        else if(c[i-1] == 0 && c[i] != 0) {
            c[i-1] = c[i];
            c[i] = 0;
        }
    }

	for(int i=SIDE-1, j=SIDE-1; i >= 0; i--) {
		if(c[i] != 0 && c[j] == 0) {
			c[j] = c[i];
			c[i] = 0;
		}
		if(c[j] != 0)
			j--;
	}
}

// Check winning condition
int has2048() {
	for(int i=0; i < SIDE; i++)
		for(int j=0; j < SIDE; j++)
			if(arr[i][j] == 2048)
				return 1;
	return 0;
}

// Check if there is any move available
int hasMovesLeft() {
	for(int i=0; i < SIDE; i++)
		for(int j=0; j < SIDE; j++)
			if(arr[i][j] == 0 || i > 0 && arr[i][j] == arr[i-1][j] || j > 0 && arr[i][j] == arr[i][j-1])
				return 1;
	return 0;				
}

// Function to check if the game is over
int gameOver() {
	return (has2048() || !hasMovesLeft());
}

// Play 2048
void play2048() {
	srand(seed);
	showInstructions();

	while(!gameOver()) {
		addNumber();
		if(gameOver())
			break;

		for(;;) {
			showBoard();
			printf("----------------------------------------\n\n");

			for(int i=0; i < SIDE; i++)
				for(int j=0; j < SIDE; j++)
					prev[i][j] = arr[i][j];

			int flag = 1;
			printf("Enter your move (W,S,A,D): ");
			char move = getchar();
			while(getchar() != '\n') ;
			printf("\n");
			switch(move) {
				case 'D':
				case 'd':
					for(int i=0; i < SIDE; i++) {
						for(int j=0; j < SIDE; j++)
							c[j] = arr[i][j];
						update();
						for(int j=0; j < SIDE; j++)
							arr[i][j] = c[j];
					}
					break;
				case 'A':
				case 'a':
					for(int i=0; i < SIDE; i++) {
						for(int j=0; j < SIDE; j++)
							c[SIDE-1-j] = arr[i][j];
						update();
						for(int j=0; j < SIDE; j++)
							arr[i][j] = c[SIDE-1-j];
					}
					break;
				case 'S':
				case 's':
					for(int i=0; i < SIDE; i++) {
						for(int j=0; j < SIDE; j++)
							c[j] = arr[j][i];
						update();
						for(int j=0; j < SIDE; j++)
							arr[j][i] = c[j];
					}
					break;
				case 'W':
				case 'w':
					for(int i=0; i < SIDE; i++) {
						for(int j=0; j < SIDE; j++)
							c[SIDE-1-j] = arr[j][i];
						update();
						for(int j=0; j < SIDE; j++)
							arr[j][i] = c[SIDE-1-j];
					}
					break;
				default:
					printf("Invalid input! Please enter W, S, A, or D.\n");
					flag = 0;
			}
			if(!flag)
				continue;
			flag = 0;
			for(int i=0; i < SIDE; i++)
				for(int j=0; j < SIDE; j++)
					flag |= prev[i][j] != arr[i][j];
			if(!flag)
				printf("Invalid move, board did not change!\n");
			else
				break;
		}
	}

	showBoard();
	printf("----------------------------------------\n\n");

	if(has2048())
		printf("You won\n");
	else
		printf("You lost!\n");
}

// Driver program
int main(int argc, char **argv) {
	if(argc == 1) {
		play2048();
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
				play2048();
			}
			else
				showUsage(argv[0]);
		}
		else
			showUsage(argv[0]);
	}

	return 0;
}
