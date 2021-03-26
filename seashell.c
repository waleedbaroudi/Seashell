#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>            //termios, TCSANOW, ECHO, ICANON
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <limits.h>

const char * sysname = "seashell";

enum return_codes {
	SUCCESS = 0,
	EXIT = 1,
	UNKNOWN = 2,
};
struct command_t {
	char *name;
	bool background;
	bool auto_complete;
	int arg_count;
	char **args;
	char *redirects[3]; // in/out redirection
	struct command_t *next; // for piping
};
/**
 * Prints a command struct
 * @param struct command_t *
 */
void print_command(struct command_t * command)
{
	int i=0;
	printf("Command: <%s>\n", command->name);
	printf("\tIs Background: %s\n", command->background?"yes":"no");
	printf("\tNeeds Auto-complete: %s\n", command->auto_complete?"yes":"no");
	printf("\tRedirects:\n");
	for (i=0;i<3;i++)
		printf("\t\t%d: %s\n", i, command->redirects[i]?command->redirects[i]:"N/A");
	printf("\tArguments (%d):\n", command->arg_count);
	for (i=0;i<command->arg_count;++i)
		printf("\t\tArg %d: %s\n", i, command->args[i]);
	if (command->next)
	{
		printf("\tPiped to:\n");
		print_command(command->next);
	}


}
/**
 * Release allocated memory of a command
 * @param  command [description]
 * @return         [description]
 */
int free_command(struct command_t *command)
{
	if (command->arg_count)
	{
		for (int i=0; i<command->arg_count; ++i)
			free(command->args[i]);
		free(command->args);
	}
	for (int i=0;i<3;++i)
		if (command->redirects[i])
			free(command->redirects[i]);
	if (command->next)
	{
		free_command(command->next);
		command->next=NULL;
	}
	free(command->name);
	free(command);
	return 0;
}
/**
 * Show the command prompt
 * @return [description]
 */
int show_prompt()
{
	char cwd[1024], hostname[1024];
    gethostname(hostname, sizeof(hostname));
	getcwd(cwd, sizeof(cwd));
	printf("%s@%s:%s %s$ ", getenv("USER"), hostname, cwd, sysname);
	return 0;
}
/**
 * Parse a command string into a command struct
 * @param  buf     [description]
 * @param  command [description]
 * @return         0
 */
int parse_command(char *buf, struct command_t *command)
{
	const char *splitters=" \t"; // split at whitespace
	int index, len;
	len=strlen(buf);
	while (len>0 && strchr(splitters, buf[0])!=NULL) // trim left whitespace
	{
		buf++;
		len--;
	}
	while (len>0 && strchr(splitters, buf[len-1])!=NULL)
		buf[--len]=0; // trim right whitespace

	if (len>0 && buf[len-1]=='?') // auto-complete
		command->auto_complete=true;
	if (len>0 && buf[len-1]=='&') // background
		command->background=true;

	char *pch = strtok(buf, splitters);
	command->name=(char *)malloc(strlen(pch)+1);
	if (pch==NULL)
		command->name[0]=0;
	else
		strcpy(command->name, pch);

	command->args=(char **)malloc(sizeof(char *));

	int redirect_index;
	int arg_index=0;
	char temp_buf[1024], *arg;
	while (1)
	{
		// tokenize input on splitters
		pch = strtok(NULL, splitters);
		if (!pch) break;
		arg=temp_buf;
		strcpy(arg, pch);
		len=strlen(arg);

		if (len==0) continue; // empty arg, go for next
		while (len>0 && strchr(splitters, arg[0])!=NULL) // trim left whitespace
		{
			arg++;
			len--;
		}
		while (len>0 && strchr(splitters, arg[len-1])!=NULL) arg[--len]=0; // trim right whitespace
		if (len==0) continue; // empty arg, go for next

		// piping to another command
		if (strcmp(arg, "|")==0)
		{
			struct command_t *c=malloc(sizeof(struct command_t));
			int l=strlen(pch);
			pch[l]=splitters[0]; // restore strtok termination
			index=1;
			while (pch[index]==' ' || pch[index]=='\t') index++; // skip whitespaces

			parse_command(pch+index, c);
			pch[l]=0; // put back strtok termination
			command->next=c;
			continue;
		}

		// background process
		if (strcmp(arg, "&")==0)
			continue; // handled before

		// handle input redirection
		redirect_index=-1;
		if (arg[0]=='<')
			redirect_index=0;
		if (arg[0]=='>')
		{
			if (len>1 && arg[1]=='>')
			{
				redirect_index=2;
				arg++;
				len--;
			}
			else redirect_index=1;
		}
		if (redirect_index != -1)
		{
			command->redirects[redirect_index]=malloc(len);
			strcpy(command->redirects[redirect_index], arg+1);
			continue;
		}

		// normal arguments
		if (len>2 && ((arg[0]=='"' && arg[len-1]=='"')
			|| (arg[0]=='\'' && arg[len-1]=='\''))) // quote wrapped arg
		{
			arg[--len]=0;
			arg++;
		}
		command->args=(char **)realloc(command->args, sizeof(char *)*(arg_index+1));
		command->args[arg_index]=(char *)malloc(len+1);
		strcpy(command->args[arg_index++], arg);
	}
	command->arg_count=arg_index;
	return 0;
}
void prompt_backspace()
{
	putchar(8); // go back 1
	putchar(' '); // write empty over
	putchar(8); // go back 1 again
}
/**
 * Prompt a command from the user
 * @param  buf      [description]
 * @param  buf_size [description]
 * @return          [description]
 */
int prompt(struct command_t *command)
{
	int index=0;
	char c;
	char buf[4096];
	static char oldbuf[4096];

    // tcgetattr gets the parameters of the current terminal
    // STDIN_FILENO will tell tcgetattr that it should write the settings
    // of stdin to oldt
    static struct termios backup_termios, new_termios;
    tcgetattr(STDIN_FILENO, &backup_termios);
    new_termios = backup_termios;
    // ICANON normally takes care that one line at a time will be processed
    // that means it will return if it sees a "\n" or an EOF or an EOL
    new_termios.c_lflag &= ~(ICANON | ECHO); // Also disable automatic echo. We manually echo each char.
    // Those new settings will be set to STDIN
    // TCSANOW tells tcsetattr to change attributes immediately.
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);


    //FIXME: backspace is applied before printing chars
	show_prompt();
	int multicode_state=0;
	buf[0]=0;
  	while (1)
  	{
		c=getchar();
		// printf("Keycode: %u\n", c); // DEBUG: uncomment for debugging

		if (c==9) // handle tab
		{
			buf[index++]='?'; // autocomplete
			break;
		}

		if (c==127) // handle backspace
		{
			if (index>0)
			{
				prompt_backspace();
				index--;
			}
			continue;
		}
		if (c==27 && multicode_state==0) // handle multi-code keys
		{
			multicode_state=1;
			continue;
		}
		if (c==91 && multicode_state==1)
		{
			multicode_state=2;
			continue;
		}
		if (c==65 && multicode_state==2) // up arrow
		{
			int i;
			while (index>0)
			{
				prompt_backspace();
				index--;
			}
			for (i=0;oldbuf[i];++i)
			{
				putchar(oldbuf[i]);
				buf[i]=oldbuf[i];
			}
			index=i;
			continue;
		}
		else
			multicode_state=0;

		putchar(c); // echo the character
		buf[index++]=c;
		if (index>=sizeof(buf)-1) break;
		if (c=='\n') // enter key
			break;
		if (c==4) // Ctrl+D
			return EXIT;
  	}
  	if (index>0 && buf[index-1]=='\n') // trim newline from the end
  		index--;
  	buf[index++]=0; // null terminate string

  	strcpy(oldbuf, buf);

  	parse_command(buf, command);

  	// print_command(command); // DEBUG: uncomment for debugging

    // restore the old settings
    tcsetattr(STDIN_FILENO, TCSANOW, &backup_termios);
  	return SUCCESS;
}
int process_command(struct command_t *command);

//-----Helper-Functions----------------------

int indexOf(char* str, char charToFind, int n);
char* substring(char* str, int from,  int to);
void tokenizer(char delimeter, char* lines[], char* path, int numOfLines);
void executeCommand(char* command, char** params);

// shortdir jump declarations //
void jump(char* input);
char *homedir; // home directory on the current machine (to place the shortdirs in)
char dirsPath[PATH_MAX]; // file path of the text file stroing shortdirs
////
char customCommandDir[PATH_MAX]; // Path to custom command executables

int main()
{
  getcwd(customCommandDir, sizeof(customCommandDir));
	// set home and shortdir paths
   homedir = getenv("HOME");
   strcpy(dirsPath, homedir);
   strcat(dirsPath, "/Documents/dirs.txt");

	while (1)
	{
		struct command_t *command=malloc(sizeof(struct command_t));
		memset(command, 0, sizeof(struct command_t)); // set all bytes to 0

		int code;
		code = prompt(command);
		if (code==EXIT) break;

		code = process_command(command);
		if (code==EXIT) break;

		free_command(command);
	}

	printf("\n");
	return 0;
}

int process_command(struct command_t *command)
{
	int r;
	if (strcmp(command->name, "")==0) return SUCCESS;

	if (strcmp(command->name, "exit")==0)
		return EXIT;

	if (strcmp(command->name, "cd")==0)
	{
		if (command->arg_count > 0)
		{
			r=chdir(command->args[0]);
			if (r==-1)
				printf("-%s: %s: %s\n", sysname, command->name, strerror(errno));
			return SUCCESS;
		}
	}

	if ((strcmp(command->name, "shortdir")==0) && (strcmp(command->args[0], "jump")==0))
	{
		jump(command->args[1]);
		return SUCCESS;
	}

	pid_t pid=fork();
	if (pid==0) // child
	{
		/// This shows how to do exec with environ (but is not available on MacOs)
	    // extern char** environ; // environment variables
		// execvpe(command->name, command->args, environ); // exec+args+path+environ

		/// This shows how to do exec with auto-path resolve
		// add a NULL argument to the end of args, and the name to the beginning
		// as required by exec

		// increase args size by 2
		command->args=(char **)realloc(
			command->args, sizeof(char *)*(command->arg_count+=2));

		// shift everything forward by 1
		for (int i=command->arg_count-2;i>0;--i)
			command->args[i]=command->args[i-1];

		// set args[0] as a copy of name
		command->args[0]=strdup(command->name);
		// set args[arg_count-1] (last) to NULL
		command->args[command->arg_count-1]=NULL;

		//		execvp(command->name, command->args); // exec+args+path
		executeCommand(command->name, command->args);
		exit(0);
		/// TODO: do your own exec with path resolving using execv()
	}
	else
	{
		if (!command->background)
			wait(0); // wait for child process to finish
		return SUCCESS;
	}

	// TODO: your implementation here

	printf("-%s: %s: command not found\n", sysname, command->name);
	return UNKNOWN;
}

/**
 * Finds the number of occurances of a character in a given string.
 * @param  str         A string to find occurances of a given character in.
 * @param  charToFind  A character to find the occurances of in a given string.
 * @return             Returns the number of occurances of a character in a given string.
 */
int numberOfOccurances(char* str, char charToFind){
  int count = 0;
  char c = str[0];
  
  while(c != '\0'){
    if(c == charToFind)
      count++;
    c = *(str++);
  }
  return count;
}

//----Shortdir Implementation----------------------------
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


//----Helper-Functions----------------------------

/**
 * Finds the index of a given character in a given string.
 * @param  str         A string to extract an index of a given character from.
 * @param  charToFind  A character whose index is to be returned.
 * @param  n           The number of the occurance of a given character in the given string.
 * @return             Returns the index of a character with n^th occurance in a given string..
 */
int indexOf(char* str, char charToFind, int n){
  char c = str[0];  
  for(int i = 0; i < strlen(str); i++){
    if(charToFind == str[i]){
      if(n == 1)
	return i;
      else
	n--;
    }
  }
  return -1;
}

/**
 * Given a string, start and end indecies, returns a substring according to the indecies.
 * @param  str      A string to extract a substring from.
 * @param  from     The index of the beginning of the substring.
 * @param  to       The index of the end of the substring.
 * @return          Returns a substring of a given string.
 */
char* substring(char* str, int from,  int to){
  char* sub = malloc(strlen(str) * sizeof(char) + 1);
  int j = 0;
  for(int i = from; i < to; i++, j++){
    sub[j] = str[i];
  }
  sub[j + 1] = '\0'; 
  return sub;
}

/**
 * Tokenizes a given string into a given array.
 * @param  delimeter   The delimeter that the given array is to be tokenized according to.
 * @param  lines       The array to add the tokens from the given string in. 
 * @param  path        A string to be tokenized.
 * @param  numOfLines  The number of tokens.
 */
void tokenizer(char delimeter, char* lines[], char* path, int numOfLines){
  int low = 0;
  int high = indexOf(path, delimeter, 1);

  int occuranceNumber = 1;
  char* line;
  for(int i = 0; i < numOfLines; i++){
    if(low == -1)
      break;

    lines[i] = substring(path, low, high);
    
    low = indexOf(path, delimeter, occuranceNumber) + 1; // In order to exclude the dilimeter.
    high = indexOf(path, delimeter, occuranceNumber + 1);

    if(occuranceNumber <= numOfLines)
      occuranceNumber++;
      }
  }

/**
 * Executes a given command.
 * @param  command  A command to be executed.
 * @param  params   The arguments for the execv() methods.
 */
void executeCommand(char* command, char** params){
  
  char* str = getenv("PATH");
  int numberOfPaths = numberOfOccurances(str, ':');
  char token = ':';
  
  char* paths[numberOfPaths + 1]; 
  tokenizer(token, paths, str, numberOfPaths);
  
  for(int i = 0; i < numberOfPaths; i++){
    strcat(paths[i], "/");
    strcat(paths[i], command);
  }
  strcpy(paths[numberOfPaths], customCommandDir);
  
  strcat(paths[numberOfPaths], "/");
  strcat(paths[numberOfPaths], command);
  
  for(int i = 0; i < numberOfPaths + 1; i++){
    params[0] = strdup(paths[i]);
    execv(paths[i], params);
  }
}
