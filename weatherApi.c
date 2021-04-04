#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <ctype.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void printfWeather(char* weatherState, double tempreture, double humidity);
void printWeatherIcon(char* weatherState);
char* getTempColor(double tempreture);
char* wordToLower(char* word);
int contains(char* word, char* wordToCheck);
char* getHumidityColor(double humidity);

char* myData;
size_t dataLen;

size_t dataWriter(char *dataBuffer, size_t size, size_t items, void* ptr)
{
    dataLen = dataLen + (size*items);
    myData = (char *) realloc(myData, dataLen);
    strcat(myData, dataBuffer);
    return size*items;
}

void printWeather() {

  char* humPart = strstr(myData, "humidity");
  strtok(humPart, ":");
  char *valuePart = strtok(NULL, ":");
  char* humidity = strtok(valuePart, ",");
  
  char* condPart = strstr(myData, "text");
  strtok(condPart, "\"");
  strtok(NULL, "\"");
  
  char* weatherCondition =  strtok(NULL, "\"");
  char* tempPart = strstr(myData, "temp_c");
  strtok(tempPart, ":");
  char *degPart = strtok(NULL, ":");
  char* tempreture = strtok(degPart, ",");

  char *tempPtr;
  char *humPtr;
  
  printfWeather(weatherCondition, strtod(tempreture, &tempPtr), strtod(humidity, &humPtr));
  
}

int main(int argc, char* argv[])
{
  CURL *curl;
  CURLcode res;

  curl = curl_easy_init();
  if(!curl) { // failed to initialize curl
    fprintf(stderr, "curl initialization failed\n");
    return EXIT_FAILURE;
  }
    // initialize empty data string
    dataLen = 10;
    myData = (char *) malloc(dataLen);
    // set curl options
    curl_easy_setopt(curl, CURLOPT_URL, "https://weatherapi-com.p.rapidapi.com/forecast.json?q=Istanbul&days=3");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dataWriter);
    // set api headers
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "x-rapidapi-key: a4ecad74e7mshfc2cb62539f3313p11baa0jsnf1afe237d144");
    headers = curl_slist_append(headers, "x-rapidapi-host: weatherapi-com.p.rapidapi.com");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    // perform reqest and store the result code
    res = curl_easy_perform(curl);

    if(res != CURLE_OK) {
      printf("Failed to fetch weather.\n");
      return 1;
    }

    printWeather();
    free(myData);


    /* always cleanup */
    curl_easy_cleanup(curl);
  return 0;
}

void printfWeather(char* weatherState, double tempreture, double humidity){

    printf("+-------------------------------------------------------------------+\n");
    printf("| Weather: %s | Tempreture: %s%.2f%s C | Humidity:%s %.1f%%%s  |\n", weatherState, getTempColor(tempreture), tempreture, ANSI_COLOR_RESET, getHumidityColor(humidity), humidity, ANSI_COLOR_RESET);
    printf("+-------------------------------------------------------------------+\n\n");

    printWeatherIcon(weatherState);

    printf("_____________________________________________________________________\n\n");
}

/**
 * Given a tempreture vlaue in Celius, returns the tempreture with a describtive color.
 */
char* getTempColor(double tempreture){
    if(tempreture <= 15)
        return ANSI_COLOR_CYAN;
    
    else if(tempreture <= 35 && tempreture > 15)
        return ANSI_COLOR_GREEN;
    
    else if(tempreture > 35)
        return ANSI_COLOR_RED;
    
    return ANSI_COLOR_RESET;
}

char* getHumidityColor(double humidity){
    if(humidity <= 20)
        return ANSI_COLOR_CYAN;

    else if(humidity > 20 && humidity <= 75)
      return ANSI_COLOR_GREEN;
    
    else if(humidity > 75)
        return ANSI_COLOR_RED;
  
    return ANSI_COLOR_RESET;
}


void printWeatherIcon(char* weatherState){

    char sunny[11][24] = {ANSI_COLOR_YELLOW"           |",
                    "    \\      |       /",
                    "     \\   ______   /",
                    "        /      \\",
                    " ____  /        \\  _____",
                    "      |          |",
                    "       \\        /",
                    "     /  \\______/  \\",
                    "    /              \\",
                    "            |",
                    "            |"ANSI_COLOR_RESET};

    char cloudy[8][33] = {ANSI_COLOR_YELLOW"                    |",
                    "              \\     |     /",
                    "               \\ _______ /",
                    "           ___  /       \\  ____",
                    "      \x1b[0m_________/_________\\_",
                    "   __(        (     )      )",
                    "  /       (           )     \\_",
                    " (___(________________________)"};

    char rainy[7][40] = {"             __________",
                    "      ______(          )___",
                    "   __(        (     )      )_",
                    "  /       (           )      \\_",
                    " (___(_________________________)",
                    ANSI_COLOR_CYAN"      \\   \\     \\   \\    \\ \\",
                    "    \\    \\    \\    \\   \\   \\"ANSI_COLOR_RESET};

    char snowy[8][32] = {"             __________",
                    "      ______(          )___",
                    "   __(        (     )      )_",
                    "  /       (           )      \\_",
                    " (___(_________________________)",
                    "      *     *      *   *    *",
                    "   *    *       * *      *  *",
                    "     *  *    *        *"};

    if(!strcmp(wordToLower(weatherState),"sunny") 
    || !strcmp(wordToLower(weatherState),"clear")){
        for(int i = 0; i < 11; i++){
            for(int j = 0; j < 24; j++){
                printf("%c", sunny[i][j]);
            }
            printf("\n");
        }
    }
        else if(!strcmp(wordToLower(weatherState),"cloudy") 
            || contains(wordToLower(weatherState), "cloud")
            || contains(wordToLower(weatherState), "fog")){
        for(int i = 0; i < 8; i++){
            for(int j = 0; j < 33; j++){
                printf("%c", cloudy[i][j]);
            }
            printf("\n");
        }
    }
        else if(contains(wordToLower(weatherState), "rain") 
            || contains(wordToLower(weatherState), "drizzle")
            || contains(wordToLower(weatherState), "shower")){
        for(int i = 0; i < 7; i++){
            for(int j = 0; j < 32; j++){
                printf("%c", rainy[i][j]);
            }
            printf("\n");
        }
    }
    else if(contains(wordToLower(weatherState), "snow")){
        for(int i = 0; i < 8; i++){
            for(int j = 0; j < 32; j++){
                printf("%c", snowy[i][j]);
            }
            printf("\n");
        }
    }
    else{
        printf(ANSI_COLOR_RED"\nWeather icon unavailable\n"ANSI_COLOR_RESET);
    }
}

char* wordToLower(char* word){
    int len = strlen(word);
    char* lowercase = malloc(sizeof(char) * len); 
    int i = 0;
    for(; i < len; i++){
        lowercase[i] = tolower(word[i]);
        }    
    lowercase[i] = '\0';
    return lowercase;
}

int contains(char* word, char* wordToCheck){
    for(int i = 0, j = 0; i < strlen(word); i++){
        if(j == strlen(wordToCheck) - 1){
            return 1;
        }
        if(word[i] == wordToCheck[j]){
            j++;
        }
        else
            j = 0;
    }    
    return 0;
}
