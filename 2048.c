#include <stdio.h> 
#include <stdlib.h> 
#include <stdbool.h>
#include <time.h>
#include <conio.h>

#include "twozerofoureight.h"

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#else
#include <unistd.h>
#endif

#if defined(_WIN32) || defined(_WIN64)
#define clear() system("cls")
#else
#define clear() system("clear")
#endif

#define L 5
#define UP 72
#define DOWN 80
#define LEFT 75
#define RIGHT 77

#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN "\x1b[36m"
#define RESET "\x1b[0m"

#define SLEEP_HIGH 40

int table[L][L];

bool was_move;

struct Player {
    int x, y;
};

static void mergeUp(void);
static void moveUp(void);
static void mergeDown(void);
static void moveDown(void);
static void mergeLeft(void);
static void moveLeft(void);
static void mergeRight(void);
static void moveRight(void);
static int numberCount(int x);
static bool hasMovesLeft(void);
static inline void init(void);
static inline struct Player randomPos(void);
static inline void swap(int* a, int* b);
static inline void __sleep(unsigned int x);
static inline int isPlayer(struct Player a);
static inline void set_wasMove(void);
static inline void unset_wasMove(void);
static inline void doublePlayer(int i, int j);
static inline void resetPlayer(int i, int j);
static inline void increasePos(int* tmp);
static inline void decreasePos(int* tmp);
static inline void resetColor(void);
static inline void setColor(int x);
static void display(void);

static inline int twoOrfour(void)
{
    int x;

    x = rand() % 20;
    if (x == 1)
        return 4;

    return 2;
}

static inline void newPlayer(void)
{
    struct Player new_player;

retry:
    new_player = randomPos();
    if (isPlayer(new_player))
        goto retry;

    table[new_player.x][new_player.y] = twoOrfour();
}

static inline void getKey(void)
{
    unsigned char key;

    (void)_getch();
    key = _getch();
    switch (key) {
    case UP:
        moveUp();
        __sleep(SLEEP_HIGH);
        mergeUp();
        break;
    case DOWN:
        moveDown();
        __sleep(SLEEP_HIGH);
        mergeDown();
        break;
    case LEFT:
        moveLeft();
        __sleep(SLEEP_HIGH);
        mergeLeft();
        break;
    case RIGHT:
        moveRight();
        __sleep(SLEEP_HIGH);
        mergeRight();
        break;
    default:
        break;
    }
}

void TwoZeroFourEight_launch(void)
{
    init();

    do {
        if (was_move) {
            __sleep(SLEEP_HIGH);
            newPlayer();
            unset_wasMove();
        }

        __sleep(SLEEP_HIGH);
        display();

        __sleep(SLEEP_HIGH);
        getKey();

        __sleep(SLEEP_HIGH);
        display();
    } while (hasMovesLeft());
}

static void mergeUp(void)
{
    int i, j;

    for (i = 1; i < L; i++)
        for (j = 0; j < L; j++)
            if (table[i][j] == table[i - 1][j]) {
                doublePlayer(i, j);
                resetPlayer(i - 1, j);
                moveUp();
            }
}

static void moveUp(void)
{
    int i, j, tmp;

    for (i = 1; i < L; i++)
        for (j = 0; j < L; j++)
            if (table[i][j]) {
                tmp = i - 1;
                while (!table[tmp][j] && tmp)
                    decreasePos(&tmp);
                if (table[tmp][j])
                    increasePos(&tmp);
                if (i != tmp) {
                    swap(&table[i][j], &table[tmp][j]);
                    set_wasMove();
                }
            }
}

static void mergeDown(void)
{
    int i, j;

    for (i = L - 1; i >= 1; i--)
        for (j = L - 1; j >= 0; j--)
            if (table[i][j] == table[i - 1][j]) {
                doublePlayer(i - 1, j);
                resetPlayer(i, j);
                moveDown();
            }
}

static void moveDown(void)
{
    int i, j, tmp;

    for (i = L - 2; i >= 0; i--)
        for (j = L - 1; j >= 0; j--)
            if (table[i][j]) {
                tmp = i + 1;
                while (!table[tmp][j] && tmp < L - 1)
                    increasePos(&tmp);
                if (table[tmp][j])
                    decreasePos(&tmp);
                if (i != tmp) {
                    swap(&table[i][j], &table[tmp][j]);
                    set_wasMove();
                }
            }
}

static void mergeLeft(void)
{
    int i, j;

    for (i = 0; i < L; i++)
        for (j = 1; j < L; j++)
            if (table[i][j] == table[i][j - 1]) {
                doublePlayer(i, j);
                resetPlayer(i, j - 1);
                moveLeft();
            }
}

static void moveLeft(void)
{
    int i, j, tmp;

    for (i = 0; i < L; i++)
        for (j = 1; j < L; j++)
            if (table[i][j]) {
                tmp = j - 1;
                while (!table[i][tmp] && tmp)
                    decreasePos(&tmp);
                if (table[i][tmp])
                    increasePos(&tmp);
                if (j != tmp) {
                    swap(&table[i][j], &table[i][tmp]);
                    set_wasMove();
                }
            }
}

static void mergeRight(void)
{
    int i, j;

    for (i = L - 1; i >= 0; i--)
        for (j = L - 1; j >= 1; j--)
            if (table[i][j] == table[i][j - 1]) {
                doublePlayer(i, j - 1);
                resetPlayer(i, j);
                moveRight();
            }
}

static void moveRight(void)
{
    int i, j, tmp;

    for (i = L - 1; i >= 0; i--)
        for (j = L - 2; j >= 0; j--)
            if (table[i][j]) {
                tmp = j + 1;
                while (!table[i][tmp] && tmp < L - 1)
                    increasePos(&tmp);
                if (table[i][tmp])
                    decreasePos(&tmp);
                if (j != tmp) {
                    swap(&table[i][j], &table[i][tmp]);
                    set_wasMove();
                }
            }
}

static int numberCount(int x)
{
    int i = 0;

    while (x) {
        i++;
        x /= 10;
    }

    return i;
}

static bool hasMovesLeft(void)
{
    int i, j, count = 0;

    for (i = 0; i < L; i++)
        for (j = 0; j < L; j++)
            if (!table[i][j])
                count++;

    return (count < 3) ? false : true;
}

static inline void init(void)
{
    srand((unsigned int)time(NULL));

    set_wasMove();
}


static inline struct Player randomPos(void)
{
    struct Player random;

    random.x = rand() % L;
    random.y = rand() % L;

    return random;
}

static inline void swap(int* a, int* b)
{
    int aux;

    aux = *a;
    *a = *b;
    *b = aux;
}

static inline void __sleep(unsigned int x)
{
#if defined(_WIN32) || defined(_WIN64)
    Sleep(x);
#else
    sleep(x);
#endif
}

static inline int isPlayer(struct Player a)
{
    return table[a.x][a.y];
}

static inline void set_wasMove(void)
{
    was_move = true;
}

static inline void unset_wasMove(void)
{
    was_move = false;
}

static inline void doublePlayer(int i, int j)
{
    table[i][j] *= 2;
}

static inline void resetPlayer(int i, int j)
{
    table[i][j] = 0;
}

static inline void increasePos(int* tmp)
{
    ++(*tmp);
}

static inline void decreasePos(int* tmp)
{
    --(*tmp);
}

static inline void resetColor(void)
{
    printf(RESET);
}

static inline void setColor(int x)
{
    switch (x) {
    case 2:
        printf(RED);
        break;
    case 4:
        printf(GREEN);
        break;
    case 8:
        printf(YELLOW);
        break;
    case 16:
        printf(BLUE);
        break;
    case 32:
        printf(MAGENTA);
        break;
    case 64:
        printf(CYAN);
        break;
    case 128:
        printf(RED);
        break;
    case 256:
        printf(GREEN);
        break;
    case 512:
        printf(YELLOW);
        break;
    case 1024:
        printf(BLUE);
        break;
    case 2048:
        printf(MAGENTA);
        break;
    case 4096:
        printf(CYAN);
        break;
    default:
        break;
    }
}

#if defined(_WIN32) || defined(_WIN64)
static void display(void)
{
    const unsigned char line_w = 196;
    const unsigned char line_h = 124;
    const unsigned char cross = 197;
    const unsigned char cross_up = 193;
    const unsigned char cross_down = 194;
    const unsigned char cross_left = 180;
    const unsigned char cross_right = 195;
    const unsigned char corner_up_left = 218;
    const unsigned char corner_up_right = 191;
    const unsigned char corner_down_left = 192;
    const unsigned char corner_down_right = 217;
    int i, j;

    clear();

    printf("%c%c%c", corner_up_left, line_w, line_w);
    printf("%c%c%c", line_w, line_w, line_w);
    printf("%c%c%c", line_w, line_w, cross_down);
    printf("%c%c%c", line_w, line_w, line_w);
    printf("%c%c%c", line_w, line_w, line_w);
    printf("%c%c%c", line_w, cross_down, line_w);
    printf("%c%c", line_w, line_w);
    printf("%c%c%c", line_w, line_w, line_w);
    printf("%c%c%c", line_w, cross_down, line_w);
    printf("%c%c", line_w, line_w);
    printf("%c%c%c", line_w, line_w, line_w);
    printf("%c%c%c", line_w, cross_down, line_w);
    printf("%c", line_w);
    printf("%c%c%c", line_w, line_w, line_w);
    printf("%c%c%c", line_w, line_w, corner_up_right);
    printf("\n");

    for (i = 0; i < L; i++) {
        for (j = 0; j < L; j++)
            switch (numberCount(table[i][j])) {
            case 2:
                printf("%c   ", line_h);
                setColor(table[i][j]);
                printf("%d  ", table[i][j]);
                resetColor();
                break;
            case 3:
                printf("%c   ", line_h);
                setColor(table[i][j]);
                printf("%d ", table[i][j]);
                resetColor();
                break;
            case 4:
                printf("%c  ", line_h);
                setColor(table[i][j]);
                printf("%d", table[i][j]);
                resetColor();
                break;
            default:
                if (!table[i][j])
                    printf("%c   O   ", line_h);
                else {
                    printf("%c   ", line_h);
                    setColor(table[i][j]);
                    printf("%d   ", table[i][j]);
                    resetColor();
                }
                break;
            }

        printf("%c", line_h);
        printf("\n");
        printf("%c%c%c", ((i == L - 1) ? corner_down_left : cross_right), line_w, line_w);
        printf("%c%c%c", line_w, line_w, line_w);
        printf("%c%c%c", line_w, line_w, ((i == L - 1) ? cross_up : cross));
        printf("%c%c%c", line_w, line_w, line_w);
        printf("%c%c%c", line_w, line_w, line_w);
        printf("%c%c%c", line_w, ((i == L - 1) ? cross_up : cross), line_w);
        printf("%c%c%c", line_w, line_w, line_w);
        printf("%c%c%c", line_w, line_w, line_w);
        printf("%c%c%c", ((i == L - 1) ? cross_up : cross), line_w, line_w);
        printf("%c%c%c", line_w, line_w, line_w);
        printf("%c%c", line_w, line_w);
        printf("%c%c%c", ((i == L - 1) ? cross_up : cross), line_w, line_w);
        printf("%c%c%c", line_w, line_w, line_w);
        printf("%c%c%c", line_w, line_w, ((i == L - 1) ? corner_down_right : cross_left));
        printf("\n");
    }
}
#else
static void display(void)
{
    int i, j;

    clear();

    for (i = 0; i < L; i++) {
        for (j = 0; j < L; j++)
            printf("%d ", table[i][j]);
        printf("\n");
    }
}
#endif