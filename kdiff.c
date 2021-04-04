#include<stdio.h>
#include<string.h>
#include<stdlib.h>

int isTextFile(char *file);
void diffText(char *first, char* second);
void diffBinary(char *first, char* second);

int main(int argc, char *argv[]) {
    if (strcmp(argv[1], "-a") == 0) // flag a -> text diff
        diffText(argv[2], argv[3]);
    else if (strcmp(argv[1], "-b") == 0) // flag b -> binary diff
        diffBinary(argv[2], argv[3]);
    else // no flag -> text diff
        diffText(argv[1], argv[2]);

    return 0;
}


/**
 * checks whether a given path points to a text file
 * @param file the path to be checked 
 */
int isTextFile(char *file) {
    char *temp = malloc(sizeof(file));
    strcpy(temp, file);
    strtok(temp, ".");
    return !(strcmp(strtok(NULL, "."), "txt"));
}

void diffText(char *first, char* second) {
    printf("[Text kdiff]\n");
    if (!isTextFile(first) || !isTextFile(second))
    {
        printf("ERROR: This mode only comapres text files (extension .txt)\n");
        return;
    }
    
    // open the two input files for reading
    FILE *file1 = fopen(first, "r");
    FILE *file2 = fopen(second, "r");

    // counter variables for the number of lines read and differences found
    int differences = 0, lineCount = 1;
    // char buffers to store the lines
    char line1 [255];
    char line2 [255];

    while (1)
    {
        // read a line from both files
        fgets(line1, 255, file1);
        fgets(line2, 255, file2);
        // if both files ended, stop
        if (feof(file1) && feof(file2))
            break;
        // if file 1 ended, indicate it an count a difference
        if (feof(file1))
        {
           printf("%s: file ended.\n", first);
           printf("%s: Line %d: %s\n", second, lineCount++, line2);
           differences++;
           continue;
        }
        // if file 2 ended, indicate it an count a difference
        if (feof(file2))
        {
           printf("%s: Line %d: %s", first, lineCount++, line1);
           printf("%s: file ended.\n\n", second);
           differences++;
           continue;
        }

        if (strcmp(line1, line2) != 0) // different lines
        {
            differences++;
            printf("%s: Line %d: %s", first, lineCount, line1);
            printf("%s: Line %d: %s\n", second, lineCount, line2);
        }

        lineCount++;
    }

    if (differences > 0) // differences found
        printf("%d different line(s) found\n", differences);
    else // 0 differences -> identical
        printf("The two files are identical\n");

    // close files streams
    fclose(file1);
    fclose(file2);
}

void diffBinary(char *first, char* second) {
    printf("[Binary kdiff]\n");
    // open the two input files for reading in binary
    FILE *file1 = fopen(first, "rb");
    FILE *file2 = fopen(second, "rb");
    // two bytes to read into
    int byte1, byte2;

    unsigned long differences = 0;
    while (1)
    {
        // read a byte from each files
        byte1 = getc(file1);
        byte2 = getc(file2);
        // difference bytes, count difference
        if (byte2!=byte1)
        {
            differences++;
            continue;
        }
        
        // if code reaches this line, then the two bytes are equal
        if (byte1 == EOF) // if one of them is EOF, stop
            break;
    }
    
    if (differences > 0) // differences found
        printf("The two files are different in %lu bytes\n", differences);
    else // 0 differences, identical files
        printf("The two files are identical\n");
    
    // close files streams
    fclose(file1);
    fclose(file2);
}