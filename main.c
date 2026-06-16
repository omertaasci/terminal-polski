#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <windows.h>
#include <stdlib.h>

void removeNewLine(char *text)
{
    text[strcspn(text, "\r\n")] = 0;
}

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    char input[200];
    char polishWord[200];
    char englishWord[200];
    char examplePL1[400];
    char exampleEN1[400];
    char examplePL2[400];
    char exampleEN2[400];
    char line[1500];

    int mistakeCounter = 0;

    printf("==== STARAM SIĘ POPRAWIĆ SWÓJ POLSKI ====\n");

    FILE *wordFile = fopen("slowa.txt", "r");

    if (wordFile == NULL)
    {
        printf("Can't find the file!\n");
        return 0;
    }

    while (fgets(line, sizeof(line), wordFile))
    {
        removeNewLine(line);

        if (strlen(line) == 0)
        {
            continue;
        }

        char *part1 = strtok(line, "\t");
        char *part2 = strtok(NULL, "\t");
        char *part3 = strtok(NULL, "\t");
        char *part4 = strtok(NULL, "\t");
        char *part5 = strtok(NULL, "\t");
        char *part6 = strtok(NULL, "\t");

        if (
            part1 == NULL ||
            part2 == NULL ||
            part3 == NULL ||
            part4 == NULL ||
            part5 == NULL ||
            part6 == NULL
        )
        {
            continue;
        }

        strcpy(polishWord, part1);
        strcpy(englishWord, part2);
        strcpy(examplePL1, part3);
        strcpy(exampleEN1, part4);
        strcpy(examplePL2, part5);
        strcpy(exampleEN2, part6);

        system("cls");
        printf("\n----------------------------------------\n");
        printf("Polish word: %s\n\n", polishWord);

        printf("Example 1:\n");
        printf("%s  =  %s\n\n", examplePL1, exampleEN1);

        printf("Example 2:\n");
        printf("%s  =  %s\n\n", examplePL2, exampleEN2);

        printf("English meaning: ");

        fgets(input, sizeof(input), stdin);
        removeNewLine(input);

        if (strcmp(input, englishWord) == 0)
        {
            printf("\nPoprawna odpowiedź 🎉👏\n");
            _getch();
        }
        else
        {
            mistakeCounter++;

            printf("\nWRONG ❌\n");
            printf("Correct answer: %s\n", englishWord);

            _getch();
        }
    }

    fclose(wordFile);

    printf("\nFinished all words!\n");
    return 0;
}
