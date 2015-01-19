#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>


void execute(char **, int, char **);
void handle_signal(int);
int parse(char *, char **, char **, int *);
void slice(char *);

#define INPUT_STRING_SIZE 80

#define NORMAL 			00
#define REDIRECTOUTPUT 	11
#define REDIRECTINPUT 	22
#define PIPE 			33
#define BACKGROUND		44
#define APPOUT			55

void handle_signal(int signo)
		{
			// printf("\nOS@manan$");
			 fflush(stdout);
		}
typedef void (*sighandler_t)(int);



int main(int argc, char *argv[])
{
	int i, mode = NORMAL, cmdArgc;
	size_t len = INPUT_STRING_SIZE;
	char *cpt, *inputString, *cmdArgv[INPUT_STRING_SIZE], *supplement = NULL;
	inputString = (char*)malloc(sizeof(char)*INPUT_STRING_SIZE);
	signal(SIGINT, SIG_IGN);
	signal(SIGINT, handle_signal);
	char Directory[100];
	
	printf("Type quit to close\n\n");
	//WHILE TO EXECUTE COMMANDS AGAIN AND AGAIN
	while(1)
	{	
		
		mode = NORMAL;
		getcwd(Directory, 100);
		printf("%s@%s >>", getlogin(),Directory);
		getline( &inputString, &len, stdin);
		//QUIT PROGRAM
		if(strcmp(inputString, "quit\n") == 0)
			exit(0);
		cmdArgc = parse(inputString, cmdArgv, &supplement, &mode);

		//CHANGE DIRECTORY
		//USING chdir(), getcwd()
		if(strcmp(*cmdArgv, "cd") == 0)
		{
			chdir(cmdArgv[1]);
		}
		else 
			execute(cmdArgv, mode, &supplement);
	}
	return 0;
}

int parse(char *inputString, char *cmdArgv[], char **supplementPtr, int *modePtr)
{
	int cmdArgc = 0, terminate = 0;
	char *srcPtr = inputString;
	while(*srcPtr != '\0' && terminate == 0)
	{
		*cmdArgv = srcPtr;
		cmdArgc++;
		while(*srcPtr != ' ' && *srcPtr != '\t' && *srcPtr != '\0' && *srcPtr != '\n' && terminate == 0)
		{
			switch(*srcPtr)
			{
				//DIFFERENT FUNCTIONS
				
				case '&':
					*modePtr = BACKGROUND;
					break;
				case '|':
					*modePtr = PIPE;
					*cmdArgv = '\0';
					srcPtr++;
					while(*srcPtr == ' ' || *srcPtr == '\t')
						srcPtr++;
					*supplementPtr = srcPtr;
					terminate = 1;
					break;
				case '<':
					*modePtr = REDIRECTINPUT;
					*cmdArgv = '\0';
					srcPtr++;
					while(*srcPtr == ' ' || *srcPtr == '\t')
						srcPtr++;
					*supplementPtr = srcPtr;
					slice(*supplementPtr);
					terminate = 1;
					break;
				case '>':
					*modePtr = REDIRECTOUTPUT;
					*cmdArgv = '\0';
					srcPtr++;
					if(*srcPtr == '>')
					{
						*modePtr = APPOUT;
						srcPtr++;
					}
					while(*srcPtr == ' ' || *srcPtr == '\t')
						srcPtr++;
					*supplementPtr = srcPtr;
					slice(*supplementPtr);
					terminate = 1;
					break;
			}
			srcPtr++;
		}
		while((*srcPtr == '\t' || *srcPtr == '\n' || *srcPtr == ' ') && terminate == 0)
		{
			*srcPtr = '\0';
			srcPtr++;
		}
		cmdArgv++;
	}
	*cmdArgv = '\0';
	return cmdArgc;
}

void slice(char *srcPtr)
{
	while(*srcPtr != ' ' && *srcPtr != '\t' && *srcPtr != '\n')
	{
		srcPtr++;
	}
	*srcPtr = '\0';
}

void execute(char **cmdArgv, int mode, char **supplementPtr)
{
	pid_t pid, pid2;
	FILE *fp;
	int mode2 = NORMAL, cmdArgc, status1, status2;
	char *cmdArgv2[INPUT_STRING_SIZE], *supplement2 = NULL;
	int pipearr[2];
	if(mode == PIPE)
	{
		if(pipe(pipearr))					//create pipe
		{
			fprintf(stderr, "Couldn't pipe");
			exit(-1);
		}
		parse(*supplementPtr, cmdArgv2, &supplement2, &mode2);
	}
	pid = fork();
	
	if(pid == 0)
	{
		switch(mode)
		{
			//PASSING PERMISSIONS
			case REDIRECTINPUT:
				fp = fopen(*supplementPtr, "r");
				dup2(fileno(fp), 0); //duplicate file descriptors
				break;
			case REDIRECTOUTPUT:
				fp = fopen(*supplementPtr, "w+");
				dup2(fileno(fp), 1);	//duplicate file descriptors
				break;
			
			case PIPE:
				close(pipearr[0]);		//INPUT CLOSE
				dup2(pipearr[1], fileno(stdout));	//duplicate file descriptors
				close(pipearr[1]);
				break;
				case APPOUT:
				fp = fopen(*supplementPtr, "a");
				dup2(fileno(fp), 1);		//duplicate file descriptors
				break;
		}
		execvp(*cmdArgv, cmdArgv);
	}
	else if( pid < 0)
	{
		printf("SOme Error");
		exit(-1);
	}
	else
	{
		if(mode == BACKGROUND)
					;
		else if(mode == PIPE)
		{
			waitpid(pid, &status1, 0);		//WAIT FOR P1 TO FINISH
			pid2 = fork();
			if(pid2 < 0)
			{
				printf("error in forking");
				exit(-1);
			}
			else if(pid2 == 0)
			{
				close(pipearr[1]);		//OUTPUT CLOSE
				dup2(pipearr[0], fileno(stdin));	//duplicate file descriptors
				close(pipearr[0]);
				execvp(*cmdArgv2, cmdArgv2);
			}
			else
			{
				close(pipearr[0]);
				close(pipearr[1]);
			}
		}
		else
			waitpid(pid, &status1, 0);
	}
}
