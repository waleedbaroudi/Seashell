#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>


char *homedir; // home directory on the current machine (to place the shortdirs in)
char schedsPath[PATH_MAX]; // file path of the text file stroing shortdirs

typedef struct event {
    char* title;
    char* description;
    char* time;
    char* date;
    int shouldRemind;
} Event;

Event* makeEvent(char* title,
    char* description,
    char* time,
    char* date,
    char* shouldRemind) {

        Event *event = malloc(sizeof(Event));
        event->title = title;
        event->description = description;
        event->time = time;
        event->date = date;
        event->shouldRemind = ((shouldRemind != NULL) && (strcmp(shouldRemind, "-r") == 0)) ? 1 : 0;

        return event;
}

void schedule(Event *event) {
    Event e = *event;
    // printf("Title: %s\nDescription: %s\nTime: %s\nDate: %s\nRemind? %s\n",
    // e.title, e.description, e.time, e.date, (e.shouldRemind == 1) ? "yes" : "no");

    char *hour = strtok(e.time, ".:-"); // split at any of these delimiters
    hour = (strcmp(hour, "all") == 0) ? "*" : hour; // replace with * if user chose 'all'

    char *minute = strtok(NULL, ".:-");

    char *day = strtok(e.date, "-/");
    day = (strcmp(day, "all") == 0) ? "*" : day;

    char *month = strtok(NULL, "-/");
    month = (strcmp(month, "all") == 0) ? "*" : month;

    // write to file
    FILE *schedsFile = fopen(schedsPath, "a");
    fprintf(schedsFile, "%s %s %s %s * XDG_RUNTIME_DIR=/run/user/$(id -u) notify-send '%s' '%s'\n",
         minute, 
         hour, 
         day, 
         month, 
         e.title, 
         e.description); // write crontab job

    if (e.shouldRemind)
    {
        ///TODO: add crontab for 10 mins earlier.
    }
    
    fclose(schedsFile);

    printf("Scheduling...\n");
    
    char *argv[3];
      argv[0] = "crontab";
      argv[1] = schedsPath;
      argv[2] = NULL;
      execvp("crontab", argv);
}

int main(int argc, char *argv[]) {

    // set home and schedsdir paths
    homedir = getenv("HOME");
    strcat(homedir, "/Documents");
    strcpy(schedsPath, homedir);
    strcat(schedsPath, "/scheds.txt");

    char* option = argv[1];

    if (strcmp(option, "schedule") == 0)
    {
        Event *event = makeEvent(argv[2], argv[3], argv[4], argv[5], argv[6]);
        schedule(event);
        return 0;
    }
    


    return 1;
}