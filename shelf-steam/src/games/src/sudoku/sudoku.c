// C program to implement 2048 game

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

int seed = 42;
int arr[9][9] = {{1,2,3,4,5,6,7,8,9},
	{4,5,6,7,8,9,1,2,3},
	{7,8,9,1,2,3,4,5,6},
	{2,3,4,5,6,7,8,9,1},
	{5,6,7,8,9,1,2,3,4},
	{8,9,1,2,3,4,5,6,7},
	{3,4,5,6,7,8,9,1,2},
	{6,7,8,9,1,2,3,4,5},
	{9,1,2,3,4,5,6,7,8}};
int pos;

// Show the help description
void showHelp(char *str) {
	printf("%s is a logic-based, combinatorial number-placement puzzle. The objective is to fill a 9x9 grid with digits so that each column, each row, and each of the nine 3x3 subgrids that compose the grid contains all of the digits from 1 to 9.\n", str);
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
	printf("\nSudoku\n\n");
	printf("----------------------------------------\n\n");
	printf("Move through cells using W, S, A, and D (parentheses mark the current position) and add digits [1-9] to empty positions!\n\n");
	printf("----------------------------------------\n\n");
}

// Function to print the game board
void showBoard() {
    for(int i=0, k=0; i < 9; i++) {
		if(i%3 == 0)
			printf("+---------+---------+---------+\n");
        for(int j=0; j < 9; j++, k++) {
			if(j%3 == 0)
				printf("|");
            if(arr[i][j] > 0) {
				if(k != pos)
					printf(" %d ", arr[i][j]);
				else
					printf("(%d)", arr[i][j]);
			}
			else if(arr[i][j] < 0) {
				printf("<%d>", -arr[i][j]);
			}
			else {
				if(k != pos)
					printf(" . ");
				else
					printf("(.)");
			}
        }
		printf("|\n");
    }
	printf("+---------+---------+---------+\n\n");
}

// Create a random game board
void createBoard() {
	for(int i=0; i < 1000; i++) {
		int d = rand()%3*3;
		int j = rand()%3+d, k = rand()%3+d;
		if(rand()%2) {
			for(int p=0; p < 9; p++) {
				int tmp = arr[j][p];
				arr[j][p] = arr[k][p];
				arr[k][p] = tmp;
			}
		}
		else {
			for(int p=0; p < 9; p++) {
				int tmp = arr[p][j];
				arr[p][j] = arr[p][k];
				arr[p][k] = tmp;
			}
		}
	}
	pos = -1;
	for(int i=0, k=0; i < 9; i++)
		for(int j=0; j < 9; j++, k++)
			if(rand()%9 < 3) {
				arr[i][j] *= -1;
			}
			else {
				arr[i][j] = 0;
				if(pos < 0)
					pos = k;
			}
}

// Update position to a valid one
void updatePosition(int di, int dj) {
	int i=pos/9, j=pos%9;
	do {
		i += di;
		if(i < 0) i += 9;
		if(i >= 9) i -= 9;
		j += dj;
		if(j < 0) j += 9;
		if(j >= 9) j -= 9;
	} while(arr[i][j] < 0);
	pos = i*9+j;
}

// Check if there is any incosistency in the board
int isValid() {
	int count_row[9][10] = {0}, count_col[9][10] = {0}, count_blk[9][10] = {0};
	for(int i=0; i < 9; i++)
		for(int j=0; j < 9; j++) {
			count_row[i][abs(arr[i][j])]++;
			count_col[j][abs(arr[i][j])]++;
			count_blk[(i/3)*3+j/3][abs(arr[i][j])]++;
		}
	for(int i=0; i < 9; i++)
		for(int j=1; j <= 9; j++)
			if(count_row[i][j] > 1 || count_col[i][j] > 1 || count_blk[i][j] > 1)
				return 0;
	return 1;
}

// Check winning condition
int isOver() {
	for(int i=0; i < 9; i++)
		for(int j=0; j < 9; j++)
			if(arr[i][j] == 0)
				return 0;
	return 1;
}

// Play Sudoku
void playSudoku() {
	srand(seed);
	showInstructions();

	createBoard();

	while(!isOver()) {
		for(;;) {
			showBoard();
			printf("----------------------------------------\n\n");

			int flag = 1, old_value;
			printf("Enter your move (W,S,A,D,[1-9]): ");
			char move = getchar();
			while(getchar() != '\n') ;
			printf("\n");
			switch(move) {
				case 'D':
				case 'd':
					updatePosition(0, 1);
					break;
				case 'A':
				case 'a':
					updatePosition(0, -1);
					break;
				case 'S':
				case 's':
					updatePosition(1, 0);
					break;
				case 'W':
				case 'w':
					updatePosition(-1, 0);
					break;
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					old_value = arr[pos/9][pos%9];
					arr[pos/9][pos%9] = move-'0';
					if(!isValid()) {
						flag = 0;
						printf("This position cannot receive a %c!\n\n", move);
						arr[pos/9][pos%9] = old_value;
					}
					break;
				default:
					printf("Invalid input! Please enter W, S, A, D, or a digit [1-9].\n\n");
					flag = 0;
			}
			if(flag)
				break;
		}
	}

	showBoard();
	printf("----------------------------------------\n\n");

	printf("Well done!\n");
}

// Driver program
int main(int argc, char **argv) {
	if(argc == 1) {
		playSudoku();
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
				playSudoku();
			}
			else
				showUsage(argv[0]);
		}
		else
			showUsage(argv[0]);
	}

	return 0;
}
