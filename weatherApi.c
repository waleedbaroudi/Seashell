#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

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
  printf("Humidity is is: %s%%\n", strtok(valuePart, ","));

  char* condPart = strstr(myData, "text");
  strtok(condPart, "\"");
  strtok(NULL, "\"");
  printf("Condition is: %s\n", strtok(NULL, "\""));

  char* tempPart = strstr(myData, "temp_c");
  strtok(tempPart, ":");
  char *degPart = strtok(NULL, ":");
  printf("Temprature is: %s degrees\n", strtok(degPart, ","));
}

int main()
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