all: bin/sudoku bin/2048 bin/tictactoe

bin/sudoku: src/sudoku/sudoku.c
	gcc $< -o $@ -std=c99

bin/2048: src/2048/2048.c
	gcc $< -o $@ -std=c99

bin/tictactoe: src/tictactoe/tictactoe.c
	gcc $< -o $@ -std=c99
