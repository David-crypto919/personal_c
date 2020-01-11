#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include "hangman.h"

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

#define UNDERLINE '_'
#define NUM_WORDS 7

static inline void init(char** string, char** array, char** used_array, int* len, bool* dyn);
static bool wasSaid(char* used_array, char charac, int i);
static inline int getRandom(void);
static inline char* randomWord(int* len);
static inline char* readString(int* len);
static void initArray(char** array, int len);
static inline char* allocArray(int len);
static inline char* __allocArray(void);
static inline void reallocUsed(char** used_array, int i);
static inline void freeMemory(char* string, char* array, char* used_array, bool dyn);
static bool processMove(char** array, char* string, int len, char charac);
static bool checkWin(char* array, int len);
static inline void gameOver(char* array, int life);
static inline void __sleep(unsigned int x);
static void display(char* array, char* used_array, int len, int i, int life);

void HangMan_launch(void)
{
    char* array, * string, * used_array;
    char charac;
    int len, life = 7, i = 0;
    bool ret, dyn = true;

    init(&string, &array, &used_array, &len, &dyn);

    display(array, used_array, len, i, life);
    ++i;
    do {
        (void)scanf(" %c", &charac);
        ret = wasSaid(used_array, charac, i);
        if (!ret) {
            reallocUsed(&used_array, i);
            used_array[i - 1] = charac;
            ret = processMove(&array, string, len, charac);
            if (!ret)
                --life;
            display(array, used_array, len, i, life);
            ++i;
        }
        else
            display(array, used_array, len, i - 1, life);
    } while (!checkWin(array, len) && life);

    gameOver(string, life);

    freeMemory(string, array, used_array, dyn);
}

static inline void init(char** string, char** array, char** used_array, int* len, bool* dyn)
{
    char carac;

    printf("Do you want to play vs your friend (F) or do you want a random (R) word?\n");
    (void)scanf(" %c", &carac);

    srand((unsigned int)time(0));

    if (carac == 'F' || carac == 'f') {
        *string = readString(len);
        *dyn = true;
    }
    else {
        *string = randomWord(len);
        *dyn = false;
    }

    *array = allocArray(*len);
    *used_array = __allocArray();
}

static bool wasSaid(char* used_array, char charac, int i)
{
    int j;

    for (j = 0; j < i; j++)
        if (used_array[j] == charac)
            return true;

    return false;
}

static inline int getRandom(void)
{
    return (rand() % (NUM_WORDS + 1));
}

static inline char* randomWord(int* len)
{
    char* words[] = { "fazan", "dictionar", "cuvant", "balaur", "uman", "suc", "calorifer", "telefon" };
    int num;

    num = getRandom();
    *len = strlen(words[num]);

    return words[num];
}

static inline char* readString(int* len)
{
    char* string, * tmp;

    string = (char*)malloc(100 * sizeof(char));
    if (!string)
        return NULL;

    clear();
    printf("Please input your word: ");
    (void)scanf("%s", string);
    *len = strlen(string);

    tmp = realloc(string, (*len + 1) * sizeof(char));
    if (!tmp)
        return NULL;

    string = tmp;
    string[*len] = '\0';

    return string;
}

static void initArray(char** array, int len)
{
    int i;

    for (i = 0; i < len; i++)
        (*array)[i] = UNDERLINE;
}

static inline char* allocArray(int len)
{
    char* array;

    array = (char*)malloc(len * sizeof(char));
    if (!array)
        return NULL;

    initArray(&array, len);

    return array;
}

static inline char* __allocArray(void)
{
    char* array;

    array = (char*)malloc(sizeof(char));
    if (!array)
        return NULL;

    return array;
}

static inline void reallocUsed(char** used_array, int i)
{
    char* tmp;

    tmp = realloc(*used_array, i * sizeof(char));
    if (!tmp)
        return;
    *used_array = tmp;
}

static inline void freeMemory(char* string, char* array, char* used_array, bool dyn)
{
    if (dyn)
        free(string);
    free(array);
    free(used_array);
}

static bool processMove(char** array, char* string, int len, char charac)
{
    int i;
    bool was_changed = false;

    for (i = 0; i < len; i++)
        if (string[i] == tolower(charac) || string[i] == toupper(charac)) {
            (*array)[i] = charac;
            was_changed = true;
        }

    return was_changed;
}

static bool checkWin(char* array, int len)
{
    int i;

    for (i = 0; i < len; i++)
        if (array[i] == UNDERLINE)
            return false;

    return true;
}

static inline void gameOver(char* string, int life)
{
    clear();
    if (life)
        printf("Congratulations! You won!\n");
    else
        printf("You lost!\n");
    printf("The word was: %s", string);

    __sleep(5000);
}

static inline void __sleep(unsigned int x)
{
#if defined(_WIN32) || defined(_WIN64)
    Sleep(x);
#else
    sleep(x);
#endif
}

static void display(char* array, char* used_array, int len, int i, int life)
{
    int j;

    clear();

    if (life > 0)
        printf("         +--------+\n"
            "         |        |\n"
            "         |        |\n"
            "         |        |\n"
            "         |        |\n"
            "         +--------+\n");

    if (life == 1) {
        printf("\n\n");
        printf("\n\n\n\n\n\n\n\n\n\n\n\n");
        printf("\n\n\n\n\n\n\n");
    }

    if (life > 1)
        printf("            |  |\n"
            "            |  |\n");

    if (life == 2) {
        printf("\n\n\n\n\n\n\n\n\n\n\n\n");
        printf("\n\n\n\n\n\n\n");
    }

    switch (life) {
    case 3:
        printf("        +----------+\n"
            "        |          |\n"
            "        |          |\n"
            "        |          |\n"
            "        |          |\n"
            "        |          |\n"
            "        |          |\n"
            "        |          |\n"
            "        |          |\n"
            "        |          |\n"
            "        |          |\n"
            "        +----------+\n");
        printf("\n\n\n\n\n\n\n");
        break;
    case 4:
        printf("        +----------+\n"
            "        |          |\n"
            "+-------|          |\n"
            "|       |          |\n"
            "|       |          |\n"
            "|       |          |\n"
            "|       |          |\n"
            "|       |          |\n"
            "|       |          |\n"
            "-       |          |\n"
            "        |          |\n"
            "        +----------+\n");
        printf("\n\n\n\n\n\n\n");
        break;
    case 5:
        printf("        +----------+        \n"
            "        |          |        \n"
            "+-------|          |-------+\n"
            "|       |          |       |\n"
            "|       |          |       |\n"
            "|       |          |       |\n"
            "|       |          |       |\n"
            "|       |          |       |\n"
            "|       |          |       |\n"
            "-       |          |       -\n"
            "        |          |        \n"
            "        +----------+        \n");
        printf("\n\n\n\n\n\n\n");
        break;
    case 6:
        printf("        +----------+        \n"
            "        |          |        \n"
            "+-------|          |-------+\n"
            "|       |          |       |\n"
            "|       |          |       |\n"
            "|       |          |       |\n"
            "|       |          |       |\n"
            "|       |          |       |\n"
            "|       |          |       |\n"
            "-       |          |       -\n"
            "        |          |        \n"
            "        +----------+        \n"
            "               | |          \n"
            "               | |          \n"
            "               | |          \n"
            "               | |          \n"
            "               | |          \n"
            "               | |          \n"
            "               |_|          \n");
        break;
    case 7:
        printf("        +----------+        \n"
            "        |          |        \n"
            "+-------|          |-------+\n"
            "|       |          |       |\n"
            "|       |          |       |\n"
            "|       |          |       |\n"
            "|       |          |       |\n"
            "|       |          |       |\n"
            "|       |          |       |\n"
            "-       |          |       -\n"
            "        |          |        \n"
            "        +----------+        \n"
            "          | |  | |          \n"
            "          | |  | |          \n"
            "          | |  | |          \n"
            "          | |  | |          \n"
            "          | |  | |          \n"
            "          | |  | |          \n"
            "          |_|  |_|          \n");
        break;
    default:
        break;
    }

    printf("\n");

    for (j = 0; j < len; j++)
        printf("%c ", array[j]);
    printf("       ");
    for (j = 0; j < i; j++)
        printf("%c ", used_array[j]);
}