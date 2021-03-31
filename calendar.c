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

/**
 * this method takes time and date and rewinds it 10 minutes
 */
void rewindTenMinutes(char **min, char **hour, char **day, char **month) {

    int numMin = atoi(*min);
    char newMin[3];
    if (numMin >= 10)
    {
        sprintf(newMin, "%d", numMin-10);
        *min = newMin;
        return;
    }

    sprintf(newMin, "%d", 60+(numMin-10));
    *min = newMin;

    if (strcmp(*hour, "all") == 0) // an hourly reminder
        return;

    int numHour = atoi(*hour);
    char newHour[3];
    if (numHour > 0)
    {
        sprintf(newHour, "%d", numHour-1);
        *hour = newHour;
        return;
    }

    sprintf(newHour, "%d", 11);
    *hour = newHour;

    if (strcmp(*day, "all") == 0) // a daily reminder
        return;

    int numDay = atoi(*day);
    char newDay[3];
    if (numDay > 1)
    {
        sprintf(newDay, "%d", numDay-1);
        *day = newDay;
        return;
    }

    sprintf(newDay, "%d", 30); // assuming all months are 30 days
    *day = newDay;

    if (strcmp(*month, "all") == 0) // a monthly reminder
        return;

    int numMonth = atoi(*month);
    char newMonth[3];
    if (numMonth > 1)
    {
        sprintf(newMonth, "%d", numMonth-1);
        *month = newMonth;
        return;
    }

    sprintf(newMonth, "%d", 11); // assuming all months are 30 days
    *month = newMonth;
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
         e.description); // write crontab job for the schedule

    if (e.shouldRemind)
    {
        rewindTenMinutes(&minute, &hour, &day, &month);
        fprintf(schedsFile, "%s %s %s %s * XDG_RUNTIME_DIR=/run/user/$(id -u) notify-send 'Reminder: %s' '[IN 10 MINUTES] %s'\n",
         minute, 
         hour, 
         day, 
         month, 
         e.title, 
         e.description); // write crontab job for the reminder
    } else {
        fprintf(schedsFile, "\n");
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