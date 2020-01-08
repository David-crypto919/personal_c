#include <stdio.h>
#include <stdbool.h> 
#include <string.h>
#include <time.h>

#include "tictactoe.h"

#define HAVE_STRUCT_TIMESPEC
#include <pthread.h> 

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#else
#include <unistd.h>
#endif

#ifndef max
#define max(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b;       \
})
#endif
#ifndef min
#define min(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b;       \
})
#endif

#if defined(_WIN32) || defined(_WIN64)
#define clear() system("cls")
#else
#define clear() system("clear")
#endif

#define T 3
#define MAXLINE 128
#define THREADS 8
#define FOUND 1
#define FAIL -1
#define TIED -2

#define RED "\x1b[31m"
#define CYAN "\x1b[36m"
#define RESET "\x1b[0m"

struct Game {
	int show, play;
} table[T][T];

struct Move {
	int row, col;
};

// Global variables for further expansion if wanted
int pc_carac;
bool player, pc, hard;

static int setMove(int pos);
static int minMax(int depth, bool is_max);
static int evalBoard(void);
void* find_lineW(void* _pc_carac);
void* find_lineH(void* _pc_carac);
void* find_lineD1(void* _pc_carac);
void* find_lineD2(void* _pc_carac);
static bool isMovesLeft(void);
void* isMovesLeft_M(void* args);
static int checkWinRow(void);
static int checkWinCol(void);
static inline bool checkWinDiag1(void);
static inline bool checkWinDiag2(void);
void* checkWinRow_M(void* args);
void* checkWinCol_M(void* args);
void* checkWinDiag1_M(void* args);
void* checkWinDiag2_M(void* args);
static inline int randPos(void);
static inline int allocHard_move(void);
static inline void __sleep(unsigned int);
static inline int countCheck(int count);
static inline int tableCheck(int i, int j, int _pc_carac);
static inline int __tableCheck(int i, int j, int _pc_carac);
static inline int finalRes(int i, int j);
static inline bool pcTurn(void);
static inline int not_pcCarac(void);
static inline void resetCount(int* count);
static inline void increaseCount(int* count);
static void display(void);

static int readHighscore(void)
{
	char str[MAXLINE];
	FILE* score;

	score = fopen("score1.txt", "r+");
	if (score == NULL) {
		printf("Can't open score file! Will make one...\n");
		return -1;
	}

	printf("Last highscores:\n");
	while (fgets(str, MAXLINE, score) != NULL)
		printf("%s", str);
	printf("\n");
	printf("\n");

	fclose(score);
	return 0;
}

static inline void writeHighscore(int x, int o, int ties)
{
	FILE* score;

	score = fopen("score1.txt", "w+");
	if (score == NULL) {
		printf("Can't open score file!\n");
		return;
	}

	printf("\nHIGHSCORES:\n");
	printf("X: %d\n", x);
	printf("O: %d\n", o);
	printf("Ties: %d\n", ties);

	fprintf(score, "X: %d\n", x);
	fprintf(score, "O: %d\n", o);
	fprintf(score, "Ties: %d", ties);

	fclose(score);
}

static void init(int was_run)
{
	int i, j, x = 0;
	char charac;

	clear();

	if (!was_run)
		readHighscore();

	printf("Do you want to play against the PC? 'Y' or 'N'\n");
	(void)scanf(" %c", &charac);
	if (charac == 'Y' || charac == 'y')
		pc = true;
	else
		pc = false;

	if (pc) {
		printf("\nDo you want to play vs Easy or Hard AI?\n");
		printf("Answer 'E' for Easy or 'H' for Hard\n");
		(void)scanf(" %c", &charac);
		if (charac == 'H' || charac == 'h')
			hard = true;
		else
			hard = false;
	}

	// Always X first
	player = true;
	pc_carac = 0;

	for (i = 0; i < T; i++)
		for (j = 0; j < T; j++) {
			table[i][j].show = ++x;
			table[i][j].play = 10 + x;
		}

	if (!was_run && readHighscore())
		writeHighscore(0, 0, 0);

	display();

	if (!was_run)
		srand((unsigned int)time(NULL));
}

static struct Move findBestMove(void)
{
	struct Move bestMove;
	int moveVal, cachedMove, bestVal = -1000;
	int i, j;

	bestMove.row = -1;
	bestMove.col = -1;

	for (i = 0; i < T; i++)
		for (j = 0; j < T; j++)
			if (table[i][j].play > 9) {
				cachedMove = table[i][j].play;
				table[i][j].play = pc_carac;
				moveVal = minMax(0, false);
				table[i][j].play = cachedMove;
				if (moveVal > bestVal) {
					bestMove.row = i;
					bestMove.col = j;
					bestVal = moveVal;
				}
			}

	return bestMove;
}

static int find_pcMove(void)
{
	void* ret;
	pthread_t tid[THREADS];
	int i;

	// Check to see if PC can win
	pthread_create(&tid[0], NULL, find_lineW, (void*)pc_carac);
	pthread_create(&tid[1], NULL, find_lineH, (void*)pc_carac);
	pthread_create(&tid[2], NULL, find_lineD1, (void*)pc_carac);
	pthread_create(&tid[3], NULL, find_lineD2, (void*)pc_carac);

	// Check to see if player can win and fuck them up
	pthread_create(&tid[4], NULL, find_lineW, (void*)not_pcCarac());
	pthread_create(&tid[5], NULL, find_lineH, (void*)not_pcCarac());
	pthread_create(&tid[6], NULL, find_lineD1, (void*)not_pcCarac());
	pthread_create(&tid[7], NULL, find_lineD2, (void*)not_pcCarac());

	for (i = 0; i < THREADS; i++) {
		pthread_join(tid[i], &ret);
		if ((int)ret >= 0)
			return (int)ret;
	}

	return randPos();
}

static void newMove(void)
{
	int pos, ret;

retry:
	if (!pcTurn())
		printf("\nEnter a number, %c: ", (player ? 'X' : '0'));

	if (pcTurn())
		if (hard)
			pos = allocHard_move();
		else
			pos = find_pcMove();
	else
		(void)scanf("%d", &pos);

	if (pos < 1 || pos > 9) {
		if (!pcTurn())
			printf("Invalid position '%d', please retry.\n", pos);
		goto retry;
	}

	ret = setMove(pos);
	if (ret) {
		if (!pcTurn())
			printf("Position already taken, please retry.\n");
		goto retry;
	}

	player = !player;

	display();
}

static int checkWin(void)
{
	int i;
	static int X, O, ties;
	char again;
	pthread_t tid[THREADS / 2 + 1];
	void* ret;

	pthread_create(&tid[0], NULL, checkWinRow_M, NULL);
	pthread_create(&tid[1], NULL, checkWinCol_M, NULL);
	pthread_create(&tid[2], NULL, checkWinDiag1_M, NULL);
	pthread_create(&tid[3], NULL, checkWinDiag2_M, NULL);
	pthread_create(&tid[4], NULL, isMovesLeft_M, NULL);

	for (i = 0; i < THREADS / 2 + 1; i++) {
		pthread_join(tid[i], &ret);
		if ((int)ret == FOUND)
			goto out;
		if ((int)ret == TIED) {
			++ties;
			goto tied;
		}
	}

	return 0;

out:
	player = !player;

	if (player)
		++X;
	else
		++O;
	printf("The winner is: %c\n", (player ? 'X' : '0'));

tied:
	printf("Want to play again? Y or N\n");
	(void)scanf(" %c", &again);
	if (again == 'Y' || again == 'y') {
		init(1);
		return 0;
	}

	writeHighscore(X, O, ties);
	return 1;
}

void TicTacToe_launch(void)
{
	init(0);
	do {
		newMove();
	} while (!checkWin());

	__sleep(3000);
}

static int setMove(int pos)
{
	int i, j;

	for (i = 0; i < T; i++)
		for (j = 0; j < T; j++)
			if (table[i][j].show == pos)
				if (table[i][j].play > 9)
					if (player)
						table[i][j].play = 1;
					else
						table[i][j].play = 0;
				else
					return -1;

	return 0;
}

static int minMax(int depth, bool is_max)
{
	int best, score, cached_move;
	int i, j;

	score = evalBoard();

	if (score)
		return score;

	if (!isMovesLeft())
		return 0;

	if (is_max) {
		best = -1000;

		for (i = 0; i < T; i++)
			for (j = 0; j < T; j++)
				if (table[i][j].play > 9) {
					cached_move = table[i][j].play;
					table[i][j].play = pc_carac;
					best = max(best, minMax(depth + 1, !is_max));
					table[i][j].play = cached_move;
				}
	}
	else {
		best = 1000;

		for (i = 0; i < T; i++)
			for (j = 0; j < T; j++)
				if (table[i][j].play > 9) {
					cached_move = table[i][j].play;
					table[i][j].play = not_pcCarac();
					best = min(best, minMax(depth + 1, !is_max));
					table[i][j].play = cached_move;
				}
	}

	return best;
}

static int evalBoard(void)
{
	int ret;

	ret = checkWinRow();
	if (ret >= 0)
		if (table[ret][0].play == pc_carac)
			return 10;
		else if (table[ret][0].play == not_pcCarac())
			return -10;

	ret = checkWinCol();
	if (ret >= 0)
		if (table[0][ret].play == pc_carac)
			return 10;
		else if (table[0][ret].play == not_pcCarac())
			return -10;

	ret = checkWinDiag1();
	if (ret)
		if (table[0][0].play == pc_carac)
			return 10;
		else if (table[0][0].play == not_pcCarac())
			return -10;

	ret = checkWinDiag2();
	if (ret)
		if (table[0][2].play == pc_carac)
			return 10;
		else if (table[0][2].play == not_pcCarac())
			return -10;

	return 0;
}

void* find_lineW(void* _pc_carac)
{
	int __pc_carac = (int)_pc_carac;
	int i, j, count = 0;

	for (i = 0; i < T; i++) {
		for (j = 0; j < T; j++) {
			if (__tableCheck(i, j, __pc_carac))
				increaseCount(&count);
			if (countCheck(count))
				for (j = 0; j < T; j++)
					if (tableCheck(i, j, __pc_carac))
						return (void*)finalRes(i, j);
		}
		resetCount(&count);
	}

	return (void*)FAIL;
}

void* find_lineH(void* _pc_carac)
{
	int __pc_carac = (int)_pc_carac;
	int i, j, count = 0;

	for (j = 0; j < T; j++) {
		for (i = 0; i < T; i++) {
			if (__tableCheck(i, j, __pc_carac))
				increaseCount(&count);
			if (countCheck(count))
				for (i = 0; i < T; i++)
					if (tableCheck(i, j, __pc_carac))
						return (void*)finalRes(i, j);
		}
		resetCount(&count);
	}

	return (void*)FAIL;
}

void* find_lineD1(void* _pc_carac)
{
	int __pc_carac = (int)_pc_carac;
	int i, count = 0;

	for (i = 0; i < T; i++) {
		if (__tableCheck(i, i, __pc_carac))
			increaseCount(&count);
		if (countCheck(count))
			for (i = 0; i < T; i++)
				if (tableCheck(i, i, __pc_carac))
					return (void*)finalRes(i, i);
	}

	return (void*)FAIL;
}

void* find_lineD2(void* _pc_carac)
{
	int __pc_carac = (int)_pc_carac;
	int i, j, count = 0;

	for (i = 0, j = 2; j >= 0; i++, j--) {
		if (__tableCheck(i, j, __pc_carac))
			increaseCount(&count);
		if (countCheck(count))
			for (i = 0, j = 2; j >= 0; i++, j--)
				if (tableCheck(i, j, __pc_carac))
					return (void*)finalRes(i, j);
	}

	return (void*)FAIL;
}

static bool isMovesLeft(void)
{
	int i, j;

	for (i = 0; i < T; i++)
		for (j = 0; j < T; j++)
			if (table[i][j].play > 9)
				return true;
	return false;
}

void* isMovesLeft_M(void* args)
{
	int i, j;

	for (i = 0; i < T; i++)
		for (j = 0; j < T; j++)
			if (table[i][j].play > 9)
				return (void*)FAIL;
	return (void*)TIED;
}

static int checkWinRow(void)
{
	int i;

	for (i = 0; i < T; i++)
		if (table[i][0].play == table[i][1].play && table[i][0].play == table[i][2].play)
			return i;
	return -1;
}

static int checkWinCol(void)
{
	int j;

	for (j = 0; j < T; j++)
		if (table[0][j].play == table[1][j].play && table[0][j].play == table[2][j].play)
			return j;
	return -1;
}

static inline bool checkWinDiag1(void)
{
	if (table[0][0].play == table[1][1].play && table[0][0].play == table[2][2].play)
		return true;
	return false;
}

static inline bool checkWinDiag2(void)
{
	if (table[0][2].play == table[1][1].play && table[0][2].play == table[2][0].play)
		return true;
	return false;
}

void* checkWinRow_M(void* args)
{
	int i;

	for (i = 0; i < T; i++)
		if (table[i][0].play == table[i][1].play && table[i][0].play == table[i][2].play)
			return (void*)FOUND;
	return (void*)FAIL;
}

void* checkWinCol_M(void* args)
{
	int j;

	for (j = 0; j < T; j++)
		if (table[0][j].play == table[1][j].play && table[0][j].play == table[2][j].play)
			return (void*)FOUND;
	return (void*)FAIL;
}

void* checkWinDiag1_M(void* args)
{
	if (table[0][0].play == table[1][1].play && table[0][0].play == table[2][2].play)
		return (void*)FOUND;
	return (void*)FAIL;
}

void* checkWinDiag2_M(void* args)
{
	if (table[0][2].play == table[1][1].play && table[0][2].play == table[2][0].play)
		return (void*)FOUND;
	return (void*)FAIL;
}

static inline int randPos(void)
{
	return ((rand() % 9) + 1);
}

static inline int allocHard_move(void)
{
	struct Move bestMove;

	bestMove = findBestMove();
	return finalRes(bestMove.row, bestMove.col);
}

static inline void __sleep(unsigned int x)
{
#if defined(_WIN32) || defined(_WIN64)
	Sleep(x);
#else
	sleep(x);
#endif
}

static inline int countCheck(int count)
{
	return count == 2;
}

static inline int tableCheck(int i, int j, int _pc_carac)
{
	return table[i][j].play != _pc_carac && table[i][j].play > 9;
}

static inline int __tableCheck(int i, int j, int _pc_carac)
{
	return table[i][j].play == _pc_carac;
}

static inline int finalRes(int i, int j)
{
	return (i * T + j + 1);
}

static inline bool pcTurn(void)
{
	if (!pc)
		return false;
	return player ? false : true;
}

static inline int not_pcCarac(void)
{
	return pc_carac ? 0 : 1;
}

static inline void resetCount(int* count)
{
	*count = 0;
}

static inline void increaseCount(int* count)
{
	++(*count);
}

#if defined(_WIN32) || defined(_WIN64)
static void display(void)
{
	const unsigned char line_h = 179;
	const unsigned char line_w = 196;
	const unsigned char corner_left = 195;
	const unsigned char corner_right = 180;
	const unsigned char corner_up_left = 218;
	const unsigned char corner_up_right = 191;
	const unsigned char cross_up = 194;
	const unsigned char corner_down_left = 192;
	const unsigned char corner_down_right = 217;
	const unsigned char cross_down = 193;
	const unsigned char cross = 197;
	int i, j;

#ifdef DEBUG
	printf("\n");
	printf("\n");
#else
	clear();
#endif

	printf("%c%c%c", corner_up_left, line_w, line_w);
	printf("%c%c%c", line_w, cross_up, line_w);
	printf("%c%c%c", line_w, line_w, cross_up);
	printf("%c%c%c", line_w, line_w, line_w);
	printf("%c", corner_up_right);
	printf("\n");

	for (i = 0; i < T; i++) {
		for (j = 0; j < T; j++) {
			if (table[i][j].play == 1) {
				printf("%c ", line_h);
				printf(RED);
				printf("X ");
				printf(RESET);
			}
			else if (!table[i][j].play) {
				printf("%c ", line_h);
				printf(CYAN);
				printf("O ");
				printf(RESET);
			}
			else
				printf("%c %d ", line_h, table[i][j].show);
		}

		printf("%c", line_h);
		printf("\n");
		printf("%c%c%c", ((i == T - 1) ? corner_down_left : corner_left), line_w, line_w);
		printf("%c%c%c", line_w, ((i == T - 1) ? cross_down : cross), line_w);
		printf("%c%c%c", line_w, line_w, ((i == T - 1) ? cross_down : cross));
		printf("%c%c%c", line_w, line_w, line_w);
		printf("%c", ((i == T - 1) ? corner_down_right : corner_right));
		printf("\n");
	}
}
#else
static void display(void)
{
	int i, j;
	char l = '|';

#ifdef DEBUG
	printf("\n");
	printf("\n");
#else
	clear();
#endif

	printf("-------------");
	printf("\n");
	for (i = 0; i < T; i++) {
		for (j = 0; j < T; j++) {
			if (table[i][j].play == 1)
				printf("%c X ", l);
			else if (!table[i][j].play)
				printf("%c 0 ", l);
			else
				printf("%c %d ", l, table[i][j].show);
		}
		printf("%c", l);
		printf("\n");
		printf("-------------");
		printf("\n");
	}
}
#endif