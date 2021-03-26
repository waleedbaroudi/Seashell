#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define COLOR_RED   "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_BLUE  "\x1b[34m"  
#define COLOR_RESET "\x1b[0m"

# define STR_LIMIT 1000

void replaceWords(char* lines, char* with, char* color);
char* getColor(char* color);

int main( char* argv[]){


  char filePath[] = "/home/mo0ff/Desktop/COMP 304/Project - 01/Seashell/textFile.txt";
  
  // char* word = toLower(argv[1][0]); // To convert the string into a lowercase character.
  //char* color = argv[2];
  //char* fileName = argv[3];

  FILE *file;
  file = fopen(filePath, "r");
 
  char line[STR_LIMIT];

  if(file == NULL){
    printf("Failed to open file.");
    return 1;
  }

  char fileContent[STR_LIMIT * STR_LIMIT] = "";

  while(fgets(line, STR_LIMIT, file)){
    strcat(fileContent, line);
  }
  strcat(fileContent, "\0");


  char demoWord[] = "The";
  char color[] = "r";
  
  replaceWords(fileContent, demoWord, color);

  printf("%s\n", fileContent);
  fclose(file);
  
  return 0;
}

void replaceWords(char* lines, char* with, char* color){

  const char del[2] = " ";
  char *word;

  word = strtok(lines, " :\n");

  char newLines[STR_LIMIT * STR_LIMIT] = "";
  
  while(word != NULL){

    if(!strcmp(word, with)){
      strcat(newLines, getColor(color));
      strcat(newLines, word);
      strcat(newLines, COLOR_RESET);
    }
    else{
      strcat(newLines, word);
    }
    // printf("%s\n", word);
    // printf("%d\n", strcmp(word, with));
    strcat(newLines, del);
    word = strtok(NULL, del);
  }
  
  strcat(newLines, "\0");
  strcpy(lines, newLines);
}

char* getColor(char* color){
  if(!strcmp(color, "r")){
    return COLOR_RED;
  }
  else if(!strcmp(color, "g")){
    return COLOR_GREEN;
  }
  else if(!strcmp(color, "b")){
    return COLOR_BLUE;
  }
  
  return NULL;
}






