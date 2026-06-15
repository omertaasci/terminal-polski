#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <windows.h>

int main()
{
    // UTF 8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    char input[150];
    char polishWord[150];
    char englishWord[150];
    char line[100];
    int mistakeCounter = 0;
    printf("==== STARAM SIĘ POPRAWIĆ SWÓJ POLSKI ====\n");

    FILE *wordFile = fopen("slowa.txt", "r");

    if (wordFile == NULL)
    {
        printf("Can't find the file!");
        return 0;
    }

    while (fgets(line, sizeof(line), wordFile))
    {
        sscanf(line, " %[^-] - %s", polishWord, englishWord);

        printf("\n%s   ------   ", polishWord);
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, englishWord) == 0)
        {
            printf("poprawna odpowiedź 🎉🎉👏👏");
            _getch();
        }
        if (strcmp(input, englishWord) == 1) 
        {
            mistakeCounter++;
            printf("WRONG");
            if (mistakeCounter >= 3)
            {
                sscanf(line, " %[^-] - %[^\n]", polishWord, englishWord);
                fclose(wordFile);
                return 0;
            }
            
        }
        
    }

    fclose(wordFile);
    return 0;
}