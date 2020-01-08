#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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

static inline char* readString(int* len);
static void initArray(char** array, int len);
static inline char* allocArray(int len);
static inline void freeMemory(char* string, char* array);
static bool processMove(char** array, char* string, int len, char charac);
static bool checkWin(char* array, char* string, int len);
static void display(char* array, int len, int life);

void HangMan_launch(void)
{
    char* array;
    char* string;
    char charac;
    int len, life = 7;
    bool ret;

    string = readString(&len);
    array = allocArray(len);

    display(array, len, life);
    do {
        (void)scanf(" %c", &charac);
        ret = processMove(&array, string, len, charac);
        if (!ret)
            --life;
        display(array, len, life);
    } while (!checkWin(array, string, len) && life);

    freeMemory(string, array);
}

static inline char* readString(int* len)
{
    char* string;
    char* tmp;

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
        (*array)[i] = '_';
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

static inline void freeMemory(char* string, char* array)
{
    free(array);
    free(string);
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

static bool checkWin(char* array, char* string, int len)
{
    int i;

    for (i = 0; i < len; i++)
        if (array[i] == '_')
            return false;

    return true;
}

static void display(char* array, int len, int life)
{
    int i;

    clear();

    if (life >= 1)
        printf("         +--------+\n"
            "         |        |\n"
            "         |        |\n"
            "         |        |\n"
            "         |        |\n"
            "         +--------+\n");
    if (life >= 2)
        printf("            |  |\n"
            "            |  |\n");

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

    printf("\n\n");

    for (i = 0; i < len; i++)
        printf("%c ", array[i]);
}