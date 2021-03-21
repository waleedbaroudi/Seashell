#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

char *homedir; // home directory on the current machine (to place the shortdirs in)
char dirsPath[PATH_MAX]; // file path of the text file stroing shortdirs

void delete(char* input);

/**
 * given a name, creates a shortdir entry mapping the name to the current directory.
 */
void set(char* input){
   delete(input); // to make sure there are no duplicates
   FILE *fp;
   char cwd[PATH_MAX];
   getcwd(cwd, sizeof(cwd)); // get current directory

   fp = fopen(dirsPath, "a+"); // open file in append mode

   fprintf(fp, "%s\n", input); // write name
   fprintf(fp, "%s\n\n", cwd); // write directory
   
   fclose(fp);
}

/**
 * given a name, jumps to the directory associated with this name
 */
void jump(char* input) {
   FILE *shortDirsFile;

   shortDirsFile = fopen(dirsPath, "r");

   char buff[255];
   int r;
    while (1)
    { // search for an associated directory
        fscanf(shortDirsFile, "%s", buff);
        if(feof(shortDirsFile)) // hit end of file
            break;
         if(strcmp(input, buff)==0) { // found match
            fscanf(shortDirsFile, "%s", buff);
            r=chdir(buff); // change directory
			   if (r==-1)
				   printf("error while changing directory\n");
            fclose(shortDirsFile);
            return;
         }
    }
   // loop terminated before finding a match
	printf("shortdir with the name '%s' does not exist.\n", input);
   fclose(shortDirsFile);
}

/**
 * replaces the contents of origFile with those of tempFile
 * this method is used in conjunction with 'delete' method to rewrite the contents
 * of shortdirs.txt after a deletion
 */
void replace(char* origFile, char* tempFile) {
   if (fork() == 0)
   { // fork a child to exectute 'mv'
      char *cmd = "mv";
      char *argv[4];
      argv[0] = "mv";
      argv[1] = tempFile;
      argv[2] = origFile;
      argv[3] = NULL;

      execvp(cmd, argv);
   } 
   else { /* this is important for when calling 'delete' in 'set'
      * to make sure addition and deletion don't run at the same time
      */
      wait(NULL);
   }
}

/**
 * given a name, deletes an associated shortdir with this name
 */ 
void delete(char* input) {
   FILE *fOrigin; // original shortdirst file
   FILE *fTemp; // temporary file for filtering out specified shortdir

   char dirsPath2[PATH_MAX];
   strcpy(dirsPath2, homedir);
   strcat(dirsPath2, "/dirs2.txt"); // create temp file path

   fOrigin = fopen(dirsPath, "r"); // open original file in read mode
   fTemp = fopen(dirsPath2, "w"); // open temp file in write mode

   char buff[255];
   int even = 1;
    while (1)
    {
        fscanf(fOrigin, "%s", buff);
        if(feof(fOrigin)) // hit end of file
            break;
         if(strcmp(input, buff)==0) { // found a match for deletion
            fscanf(fOrigin, "%s", buff);
            continue;
         }
         if (even)
         { // this is for printing an empty space after each shortdir entry
           fprintf(fTemp, "%s\n", buff);
           even = 0;
         } else {
            fprintf(fTemp, "%s\n\n", buff);
            even = 1;
         }
    }

   // close streams
   fclose(fOrigin);
   fclose(fTemp);
   // replace the original entries with the temporary, filtered entries
   replace(dirsPath, dirsPath2);
}

/**
 * deletes saved shortdirs
 */
void clear() {
   // opening existing file in w+ mode to truncate its size to 0 (deleting everything)
   fclose(fopen(dirsPath, "w+"));
}

/**
 * list all shortdir entries 
 */
void list() {
   FILE *shortDirsFile;
   shortDirsFile = fopen(dirsPath, "r"); // open file
   printf("\n"); // printing empty line for better presentation
   char name[255];
   char path[255];
    while (1)
    {
        fscanf(shortDirsFile, "%s", name);
         if(feof(shortDirsFile)) // stop at end of file
            break;

         fscanf(shortDirsFile, "%s", path);
         printf("%s -> %s\n", name, path);
    }
   fclose(shortDirsFile);
}

int main(int argc, char *argv[]) {
   // set home and shortdir paths
   homedir = getenv("HOME");
   strcat(homedir, "/Documents");
   strcpy(dirsPath, homedir);
   strcat(dirsPath, "/dirs.txt");

   char* op = argv[1]; // first argument is the operation (e.g. set, del)

   if(strcmp(op, "set") == 0) {
      set(argv[2]); // seconed argument is the input (which is optional)
      return 0;
   }
   if(strcmp(op, "del") == 0) {
      delete(argv[2]);
      return 0;
   }

   if(strcmp(op, "clear") == 0) {
      clear();
      return 0;
   }   

   if(strcmp(op, "list") == 0) {
      list();
      return 0;
   }

   if(strcmp(op, "jump") == 0) {
      jump(argv[2]);
      return 0;
   }

   // exit with error if an unknown operation was entered
   return -1;
}