#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


int main(int argc, char *argv[]) {

    char cronTempPath[] = "./tempcron.txt";
    char* time = argv[1];
    char* file = argv[2];


    char *hour = strtok(time, ".:-"); // split at any of these delimiters
    char *minute = strtok(NULL, ".:-");
    printf("Will play %s everday at %s:%s\n", file, hour, minute);

    // write cron job to a file
    FILE *tempCrontab = fopen(cronTempPath, "w");
    fprintf(tempCrontab, "%s %s * * * XDG_RUNTIME_DIR=/run/user/$(id -u) DISPLAY=:0.0 rhythmbox-client --play %s\n", minute, hour, file); // write crontab job
    fclose(tempCrontab);

    // add job to crontab
    if(fork() == 0) {
      char *argv[3];
      argv[0] = "crontab";
      argv[1] = cronTempPath;
      argv[2] = NULL;
      execvp("crontab", argv);
    } else {
        wait(NULL);

        char *argv[3];
        argv[0] = "rm";
        argv[1] = cronTempPath;
        argv[2] = NULL;

        execvp("rm", argv);
    }

    return 0;
}