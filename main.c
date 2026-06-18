#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#endif

#define FILE_NAME "slowa.txt"
#define STATS_FILE "stats.txt"

#define MAX_WORDS 1000
#define MAX_INPUT 200
#define MAX_WORD 200
#define MAX_EXAMPLE 400
#define MAX_LINE 1500

#define COLOR_NORMAL 7
#define COLOR_TITLE 11
#define COLOR_SUCCESS 10
#define COLOR_ERROR 12
#define COLOR_WARNING 14
#define COLOR_SOFT 8

typedef struct
{
    char polish[MAX_WORD];
    char english[MAX_WORD];
    char examplePL1[MAX_EXAMPLE];
    char exampleEN1[MAX_EXAMPLE];
    char examplePL2[MAX_EXAMPLE];
    char exampleEN2[MAX_EXAMPLE];
} Word;

typedef enum
{
    ANSWER_WRONG,
    ANSWER_EXACT,
    ANSWER_PARTIAL
} AnswerResult;

void setColor(int color)
{
#ifdef _WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
#else
    (void)color;
#endif
}

void clearScreen()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void waitForKey()
{
    setColor(COLOR_SOFT);
    printf("\nPress any key to continue...");
    setColor(COLOR_NORMAL);

#ifdef _WIN32
    _getch();
#else
    getchar();
#endif
}

void removeNewLine(char *text)
{
    text[strcspn(text, "\r\n")] = '\0';
}

void trim(char *text)
{
    int start = 0;
    int end = (int)strlen(text) - 1;

    while (text[start] && isspace((unsigned char)text[start]))
    {
        start++;
    }

    while (end >= start && isspace((unsigned char)text[end]))
    {
        text[end] = '\0';
        end--;
    }

    if (start > 0)
    {
        memmove(text, text + start, strlen(text + start) + 1);
    }
}

void safeCopy(char *destination, size_t size, const char *source)
{
    snprintf(destination, size, "%s", source);
    trim(destination);
}

int equalsIgnoreCase(const char *a, const char *b)
{
    while (*a && *b)
    {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
        {
            return 0;
        }

        a++;
        b++;
    }

    return *a == '\0' && *b == '\0';
}

void printLine()
{
    setColor(COLOR_SOFT);
    printf("------------------------------------------------------------\n");
    setColor(COLOR_NORMAL);
}

void printTitle()
{
    setColor(COLOR_TITLE);
    printf("============================================================\n");
    printf("        STARAM SIE POPRAWIC SWOJ POLSKI\n");
    printf("============================================================\n");
    setColor(COLOR_NORMAL);
}

void normalizeText(char *text)
{
    int i;
    int j = 0;
    int lastWasSpace = 1;
    char result[MAX_WORD * 2];

    trim(text);

    for (i = 0; text[i] != '\0' && j < (int)sizeof(result) - 1; i++)
    {
        unsigned char ch = (unsigned char)text[i];

        if (isalnum(ch))
        {
            result[j++] = (char)tolower(ch);
            lastWasSpace = 0;
        }
        else if (isspace(ch))
        {
            if (!lastWasSpace)
            {
                result[j++] = ' ';
                lastWasSpace = 1;
            }
        }
        else
        {
            if (!lastWasSpace)
            {
                result[j++] = ' ';
                lastWasSpace = 1;
            }
        }
    }

    if (j > 0 && result[j - 1] == ' ')
    {
        j--;
    }

    result[j] = '\0';
    snprintf(text, MAX_WORD * 2, "%s", result);
}

void removeLeadingArticle(char *text)
{
    int changed = 1;

    while (changed)
    {
        changed = 0;

        if (strncmp(text, "to ", 3) == 0)
        {
            memmove(text, text + 3, strlen(text + 3) + 1);
            changed = 1;
        }
        else if (strncmp(text, "a ", 2) == 0)
        {
            memmove(text, text + 2, strlen(text + 2) + 1);
            changed = 1;
        }
        else if (strncmp(text, "an ", 3) == 0)
        {
            memmove(text, text + 3, strlen(text + 3) + 1);
            changed = 1;
        }
        else if (strncmp(text, "the ", 4) == 0)
        {
            memmove(text, text + 4, strlen(text + 4) + 1);
            changed = 1;
        }

        trim(text);
    }
}

int textContainsWithGoodRatio(const char *correct, const char *answer)
{
    int lenCorrect = (int)strlen(correct);
    int lenAnswer = (int)strlen(answer);
    int shorter;
    int longer;
    double ratio;

    if (lenCorrect < 4 || lenAnswer < 4)
    {
        return 0;
    }

    if (strstr(correct, answer) == NULL && strstr(answer, correct) == NULL)
    {
        return 0;
    }

    shorter = lenCorrect < lenAnswer ? lenCorrect : lenAnswer;
    longer = lenCorrect > lenAnswer ? lenCorrect : lenAnswer;

    ratio = (double)shorter / longer;

    return ratio >= 0.65;
}

int tokenExists(const char *text, const char *token)
{
    char copy[MAX_WORD * 2];
    char *part;

    snprintf(copy, sizeof(copy), "%s", text);

    part = strtok(copy, " ");

    while (part != NULL)
    {
        if (strcmp(part, token) == 0)
        {
            return 1;
        }

        part = strtok(NULL, " ");
    }

    return 0;
}

int wordOverlapIsGood(const char *correct, const char *answer)
{
    char copy[MAX_WORD * 2];
    char *part;
    int totalTokens = 0;
    int matchedTokens = 0;

    snprintf(copy, sizeof(copy), "%s", correct);

    part = strtok(copy, " ");

    while (part != NULL)
    {
        if (strlen(part) >= 3)
        {
            totalTokens++;

            if (tokenExists(answer, part))
            {
                matchedTokens++;
            }
        }

        part = strtok(NULL, " ");
    }

    if (totalTokens < 2)
    {
        return 0;
    }

    return matchedTokens >= totalTokens - 1;
}

AnswerResult checkOneAnswer(const char *userAnswer, const char *correctOption)
{
    char userClean[MAX_WORD * 2];
    char correctClean[MAX_WORD * 2];

    snprintf(userClean, sizeof(userClean), "%s", userAnswer);
    snprintf(correctClean, sizeof(correctClean), "%s", correctOption);

    normalizeText(userClean);
    normalizeText(correctClean);

    if (strlen(userClean) == 0 || strlen(correctClean) == 0)
    {
        return ANSWER_WRONG;
    }

    if (strcmp(userClean, correctClean) == 0)
    {
        return ANSWER_EXACT;
    }

    removeLeadingArticle(userClean);
    removeLeadingArticle(correctClean);

    if (strcmp(userClean, correctClean) == 0)
    {
        return ANSWER_EXACT;
    }

    if (textContainsWithGoodRatio(correctClean, userClean))
    {
        return ANSWER_PARTIAL;
    }

    if (wordOverlapIsGood(correctClean, userClean))
    {
        return ANSWER_PARTIAL;
    }

    return ANSWER_WRONG;
}

AnswerResult checkAnswer(const char *userAnswer, const char *correctAnswer)
{
    char correctCopy[MAX_WORD];
    char *option;
    AnswerResult bestResult = ANSWER_WRONG;

    snprintf(correctCopy, sizeof(correctCopy), "%s", correctAnswer);

    option = strtok(correctCopy, "/;|");

    while (option != NULL)
    {
        AnswerResult result;

        trim(option);
        result = checkOneAnswer(userAnswer, option);

        if (result == ANSWER_EXACT)
        {
            return ANSWER_EXACT;
        }

        if (result == ANSWER_PARTIAL)
        {
            bestResult = ANSWER_PARTIAL;
        }

        option = strtok(NULL, "/;|");
    }

    return bestResult;
}

void getFirstCorrectOption(const char *correctAnswer, char *output, size_t size)
{
    char copy[MAX_WORD];
    char *option;

    snprintf(copy, sizeof(copy), "%s", correctAnswer);

    option = strtok(copy, "/;|");

    if (option == NULL)
    {
        snprintf(output, size, "%s", correctAnswer);
    }
    else
    {
        snprintf(output, size, "%s", option);
    }

    trim(output);
}

void showHint(const char *correctAnswer, int hintLevel)
{
    char option[MAX_WORD];
    char hint[MAX_WORD];
    int i;
    int j = 0;
    int letterIndexInWord = 0;

    getFirstCorrectOption(correctAnswer, option, sizeof(option));

    for (i = 0; option[i] != '\0' && j < MAX_WORD - 1; i++)
    {
        unsigned char ch = (unsigned char)option[i];

        if (isalnum(ch))
        {
            letterIndexInWord++;

            if (letterIndexInWord <= hintLevel)
            {
                hint[j++] = option[i];
            }
            else
            {
                hint[j++] = '-';
            }
        }
        else
        {
            hint[j++] = option[i];
            letterIndexInWord = 0;
        }
    }

    hint[j] = '\0';

    setColor(COLOR_WARNING);
    printf("\nHint %d: %s\n", hintLevel, hint);
    setColor(COLOR_SOFT);
    printf("You can type hint again for a stronger hint.\n");
    setColor(COLOR_NORMAL);
}

int parseLine(char *line, Word *word)
{
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
        return 0;
    }

    safeCopy(word->polish, sizeof(word->polish), part1);
    safeCopy(word->english, sizeof(word->english), part2);
    safeCopy(word->examplePL1, sizeof(word->examplePL1), part3);
    safeCopy(word->exampleEN1, sizeof(word->exampleEN1), part4);
    safeCopy(word->examplePL2, sizeof(word->examplePL2), part5);
    safeCopy(word->exampleEN2, sizeof(word->exampleEN2), part6);

    return 1;
}

int loadWords(Word words[])
{
    FILE *wordFile = fopen(FILE_NAME, "r");
    char line[MAX_LINE];
    char lineCopy[MAX_LINE];
    int total = 0;

    if (wordFile == NULL)
    {
        setColor(COLOR_ERROR);
        printf("Can't find the file: %s\n", FILE_NAME);
        setColor(COLOR_NORMAL);
        return -1;
    }

    while (fgets(line, sizeof(line), wordFile) && total < MAX_WORDS)
    {
        removeNewLine(line);
        trim(line);

        if (strlen(line) == 0)
        {
            continue;
        }

        snprintf(lineCopy, sizeof(lineCopy), "%s", line);

        if (parseLine(lineCopy, &words[total]))
        {
            total++;
        }
    }

    fclose(wordFile);
    return total;
}

void shuffleWords(Word words[], int total)
{
    int i;

    for (i = total - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        Word temp = words[i];
        words[i] = words[j];
        words[j] = temp;
    }
}

void printQuestion(
    Word word,
    int current,
    int total,
    int correctCounter,
    int partialCounter,
    int mistakeCounter,
    int skippedCounter,
    int hintCounter
)
{
    clearScreen();
    printTitle();

    setColor(COLOR_SOFT);
    printf("Word %d of %d\n", current, total);
    printf("Correct: %d | Partial: %d | Wrong: %d | Skipped: %d | Hints: %d\n",
           correctCounter,
           partialCounter,
           mistakeCounter,
           skippedCounter,
           hintCounter);
    setColor(COLOR_NORMAL);

    printLine();

    setColor(COLOR_WARNING);
    printf("Polish word:\n");
    setColor(COLOR_TITLE);
    printf("%s\n\n", word.polish);
    setColor(COLOR_NORMAL);

    setColor(COLOR_WARNING);
    printf("Example 1:\n");
    setColor(COLOR_NORMAL);
    printf("%s\n", word.examplePL1);
    setColor(COLOR_SOFT);
    printf("%s\n\n", word.exampleEN1);

    setColor(COLOR_WARNING);
    printf("Example 2:\n");
    setColor(COLOR_NORMAL);
    printf("%s\n", word.examplePL2);
    setColor(COLOR_SOFT);
    printf("%s\n\n", word.exampleEN2);

    printLine();

    setColor(COLOR_SOFT);
    printf("Commands: hint, skip, quit\n");
    setColor(COLOR_NORMAL);

    printf("English meaning: ");
}

void saveStats(
    int totalWords,
    int practicedWords,
    int correctCounter,
    int partialCounter,
    int mistakeCounter,
    int skippedCounter,
    int hintCounter,
    double exactScore,
    double learningScore
)
{
    FILE *statsFile = fopen(STATS_FILE, "a");
    time_t now;
    struct tm *localTime;
    char dateText[100];

    if (statsFile == NULL)
    {
        setColor(COLOR_ERROR);
        printf("\nCould not save stats to %s\n", STATS_FILE);
        setColor(COLOR_NORMAL);
        return;
    }

    now = time(NULL);
    localTime = localtime(&now);

    if (localTime != NULL)
    {
        strftime(dateText, sizeof(dateText), "%Y-%m-%d %H:%M:%S", localTime);
    }
    else
    {
        snprintf(dateText, sizeof(dateText), "%s", "Unknown date");
    }

    fprintf(statsFile, "========================================\n");
    fprintf(statsFile, "Date: %s\n", dateText);
    fprintf(statsFile, "Total words in file: %d\n", totalWords);
    fprintf(statsFile, "Practiced words: %d\n", practicedWords);
    fprintf(statsFile, "Correct answers: %d\n", correctCounter);
    fprintf(statsFile, "Partial answers: %d\n", partialCounter);
    fprintf(statsFile, "Wrong answers: %d\n", mistakeCounter);
    fprintf(statsFile, "Skipped words: %d\n", skippedCounter);
    fprintf(statsFile, "Hints used: %d\n", hintCounter);
    fprintf(statsFile, "Exact score: %.1f%%\n", exactScore);
    fprintf(statsFile, "Learning score: %.1f%%\n", learningScore);
    fprintf(statsFile, "\n");

    fclose(statsFile);

    setColor(COLOR_SUCCESS);
    printf("\nStats saved to %s\n", STATS_FILE);
    setColor(COLOR_NORMAL);
}

void printFinalResult(
    int totalWords,
    int practicedWords,
    int correctCounter,
    int partialCounter,
    int mistakeCounter,
    int skippedCounter,
    int hintCounter
)
{
    double exactScore = 0.0;
    double learningScore = 0.0;

    if (practicedWords > 0)
    {
        exactScore = ((double)correctCounter / practicedWords) * 100.0;
        learningScore = (((double)correctCounter + ((double)partialCounter * 0.5)) / practicedWords) * 100.0;
    }

    clearScreen();
    printTitle();

    setColor(COLOR_TITLE);
    printf("Finished!\n\n");
    setColor(COLOR_NORMAL);

    printLine();

    printf("Total words in file: %d\n", totalWords);
    printf("Practiced words:     %d\n", practicedWords);

    setColor(COLOR_SUCCESS);
    printf("Correct answers:     %d\n", correctCounter);

    setColor(COLOR_WARNING);
    printf("Partial answers:     %d\n", partialCounter);

    setColor(COLOR_ERROR);
    printf("Wrong answers:       %d\n", mistakeCounter);

    setColor(COLOR_SOFT);
    printf("Skipped words:       %d\n", skippedCounter);
    printf("Hints used:          %d\n", hintCounter);

    setColor(COLOR_NORMAL);
    printLine();

    setColor(COLOR_SUCCESS);
    printf("Exact score:         %.1f%%\n", exactScore);

    setColor(COLOR_WARNING);
    printf("Learning score:      %.1f%%\n", learningScore);

    setColor(COLOR_NORMAL);
    printLine();

    if (learningScore >= 90.0)
    {
        setColor(COLOR_SUCCESS);
        printf("Excellent work!\n");
    }
    else if (learningScore >= 70.0)
    {
        setColor(COLOR_WARNING);
        printf("Good job! Keep practicing.\n");
    }
    else
    {
        setColor(COLOR_ERROR);
        printf("Do not give up. Practice makes progress.\n");
    }

    setColor(COLOR_NORMAL);

    saveStats(
        totalWords,
        practicedWords,
        correctCounter,
        partialCounter,
        mistakeCounter,
        skippedCounter,
        hintCounter,
        exactScore,
        learningScore
    );
}

int main()
{
    Word words[MAX_WORDS];
    char input[MAX_INPUT];

    int totalWords;
    int i;

    int correctCounter = 0;
    int partialCounter = 0;
    int mistakeCounter = 0;
    int skippedCounter = 0;
    int hintCounter = 0;
    int practicedWords = 0;

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    srand((unsigned int)time(NULL));

    totalWords = loadWords(words);

    if (totalWords < 0)
    {
        return 1;
    }

    if (totalWords == 0)
    {
        setColor(COLOR_ERROR);
        printf("The file is empty or has no valid lines.\n");
        setColor(COLOR_NORMAL);
        return 1;
    }

    shuffleWords(words, totalWords);

    for (i = 0; i < totalWords; i++)
    {
        int hintLevel = 0;

        while (1)
        {
            AnswerResult result;

            printQuestion(
                words[i],
                i + 1,
                totalWords,
                correctCounter,
                partialCounter,
                mistakeCounter,
                skippedCounter,
                hintCounter
            );

            if (hintLevel > 0)
            {
                showHint(words[i].english, hintLevel);
                printf("\nEnglish meaning: ");
            }

            if (fgets(input, sizeof(input), stdin) == NULL)
            {
                continue;
            }

            removeNewLine(input);
            trim(input);

            if (strlen(input) == 0)
            {
                setColor(COLOR_WARNING);
                printf("\nPlease enter an answer.\n");
                setColor(COLOR_NORMAL);
                waitForKey();
                continue;
            }

            if (
                equalsIgnoreCase(input, "quit") ||
                equalsIgnoreCase(input, "q") ||
                equalsIgnoreCase(input, "exit")
            )
            {
                printFinalResult(
                    totalWords,
                    practicedWords,
                    correctCounter,
                    partialCounter,
                    mistakeCounter,
                    skippedCounter,
                    hintCounter
                );

                return 0;
            }

            if (equalsIgnoreCase(input, "hint"))
            {
                hintLevel++;
                hintCounter++;
                continue;
            }

            if (equalsIgnoreCase(input, "skip"))
            {
                skippedCounter++;
                practicedWords++;

                setColor(COLOR_WARNING);
                printf("\nSkipped.\n");
                setColor(COLOR_NORMAL);

                printf("Correct answer: ");

                setColor(COLOR_SUCCESS);
                printf("%s\n", words[i].english);
                setColor(COLOR_NORMAL);

                waitForKey();
                break;
            }

            result = checkAnswer(input, words[i].english);
            practicedWords++;

            if (result == ANSWER_EXACT)
            {
                correctCounter++;

                setColor(COLOR_SUCCESS);
                printf("\nCorrect! Great job.\n");
                setColor(COLOR_NORMAL);

                waitForKey();
                break;
            }
            else if (result == ANSWER_PARTIAL)
            {
                partialCounter++;

                setColor(COLOR_WARNING);
                printf("\nAlmost correct. I accepted it as partial.\n");
                setColor(COLOR_NORMAL);

                printf("Best answer: ");

                setColor(COLOR_SUCCESS);
                printf("%s\n", words[i].english);
                setColor(COLOR_NORMAL);

                waitForKey();
                break;
            }
            else
            {
                mistakeCounter++;

                setColor(COLOR_ERROR);
                printf("\nWrong.\n");
                setColor(COLOR_NORMAL);

                printf("Correct answer: ");

                setColor(COLOR_SUCCESS);
                printf("%s\n", words[i].english);
                setColor(COLOR_NORMAL);

                waitForKey();
                break;
            }
        }
    }

    printFinalResult(
        totalWords,
        practicedWords,
        correctCounter,
        partialCounter,
        mistakeCounter,
        skippedCounter,
        hintCounter
    );

    return 0;
}
