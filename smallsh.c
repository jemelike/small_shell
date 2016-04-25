/* include file for example */
#include "smallsh.h"
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

/*program buffers and work pointers*/
static char inpbuf[MAXBUF], tokbuf[2*MAXBUF], *ptr = inpbuf, *tok = tokbuf, special [] = {' ', '\t', '&', ';', '\n', '\0'};
char *prompt = "Command> ";
pid_t testPid;
main()
{	
	static struct sigaction act, act2, act3;
	
	act.sa_handler = end_Process;
	act2.sa_handler = stop_Process;
	act3.sa_handler = job_remove_handle;
	
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGTSTP, &act2, NULL);
	sigaction(SIGCHLD, &act3, NULL);
	
	while( userin(prompt) != EOF)
		procline();
}

/* print prompt and read a line */
int userin(char *p)
{
	int c, count;
	
	/* initialize for later routines */
	ptr = inpbuf;
	tok = tokbuf;
	
	/* display prompt */
	printf("%s", p);
	count = 0;
	
	while(1)
	{
		if((c = getchar())== EOF)
			return (EOF);
		
		if(count < MAXBUF)
			inpbuf[count++] = c;
			
		if(c == '\n' && count < MAXBUF)
		{
			inpbuf[count] = '\0';
			return count;
		}
		
		/* if line too long restart */
		if(c == '\n')
		{
			printf("smallsh: input line too long\n");
			count = 0;
			printf("%s", p);
		}
	}
}

/* get token, place into tokbuf */
int gettok(char **outptr){
	int type;
	
	/* set the outptr string to tok */
	*outptr = tok;
	
	/* strip white space from the buffer containing the tokens */
	while(*ptr == ' ' || *ptr == '\t')
		ptr++;
	
	/* set the token pointer to the first token in the buffer */
	*tok++ = *ptr;
	
	/* set the type variable depending
	 * on the token in the buffer */
	 switch(*ptr++)
	 {
		 case '\n':
			type = EOL;
			break;
		 case '&':
			type = AMPERSAND;
			break;
		 case ';':
			type = SEMICOLON;
			break;
		 default:
			type = ARG;
			/* keep reading valid ordinary characters */
			while(inarg(*ptr))
				*tok++ = *ptr++;
			break;
	 }
	 
	 *tok++ = '\0';
	 return type;
}

int inarg(char c){
	char *wrk;
	
	for(wrk = special; *wrk; wrk++){
		if(c == *wrk)
			return (0);
	}
	
	return (1);
}

int procline(void){ 			/* process input line */
	char *arg[MAXARG + 1];		/* pointer array for runcommand */
	int toktype;				/* type of token in command */
	int narg;					/* number of arguments so far */
	int type;					/* FOREGROUND or BACKGROUND */
	
	narg = 0;
	
	for(;;){ 					/* loop forever */
		/* take action according to token type  */
		switch(toktype = gettok(&arg[narg])){
			case ARG: 
				if(narg < MAXARG)
					narg++;
				break;
			case EOL:
			case SEMICOLON:
			case AMPERSAND:
				if(toktype == AMPERSAND)
					type = BACKGROUND;
				else
					type = FOREGROUND;
				
				if(narg != 0){
					arg[narg] = NULL;
					
					if(strcmp("job\0", arg[0]) == 0)
					{
						//printJobList(joblist);
					}
					else if(strcmp("bg\0",arg[0]) == 0 | strcmp("fg\0",arg[0]) ==0)
					{
						if(strcmp("bg\0",arg[0]) == 0)
							setType(BACKGROUND, (pid_t)arg[1]);
						else
							setType(FOREGROUND, (pid_t)arg[1]);
					}
					else if(strcmp("kill\0", arg[0]) == 0)
					{
						kill(SIGKILL, (pid_t)arg[1]);
					}
					else{
						runcommand(arg, type);
					}
				}
				
				if(toktype == EOL)
					return;
				
				narg = 0;
				break;
		}
	}
}

/* execute a command with optional wait */
int runcommand(char **cline, int where){
	pid_t pid;
	int status;
	
	
	switch(pid = fork()){
		case -1:
			perror("smallsh");
			return (-1);
		case 0:
			//add job to joblist
			job *new;
			
			new->jobID = ++numjob;
			new->jobID->pjob->pid = getpid();
			if(where == FOREGROUND)
				new->jobID->pjob->type = FOREGROUND;
			else
				new->jobID->pjob->type = BACKGROUND;
				
			new->jobID->pjob->running = 1;
			
			//Connect commands based on the number of pipes in the input
			
			addJob(new);
			execvp(*cline, cline);
			perror(*cline);
			exit(1);
	}
	testPid = pid;
	/* code for parent */
	/* if background process print pid and exit */
	if(where == BACKGROUND){
		printf("[Process id %d]\n", pid);
		return (0);
	}
	
	/* wait until process pid exits */
	if(waitpid(pid, &status, 0) == -1)
		return (-1);
	
	else
		return (status);
}
//use waitpid to determine whether job has finished
//sigcon for bg
void setType(int type, pid_t pid)
{
}

void end_Process()
{
	kill(SIGKILL,findFG());
}

void stop_Process()
{
	pid_t pid;
	pid = findFG();
	printf("[Stopped] %d\n", pid);
	kill(SIGTSTP,pid);
}


void addJob(struct job* new)
{
	struct job *current;
	 
	if(job_list == NULL)
		job_list = new;
	else	 
		for( current = job_list; current != NULL ;current = current->next)
			if(current->next == NULL){
				current->next = new;
				break;
			}
}

void job_remove_handle(){
	
}

pid_t findFG(){
	for(struct job* current = job_list; current != NULL; current = current->next)
		if(current->pjob->type == FOREGROUND)
			return current->pjob->pid;
}	
