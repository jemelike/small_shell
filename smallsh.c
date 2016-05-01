/* include file for example */
#include "smallsh.h"
#include "string.h"
#include <stdlib.h>

int userin(char *);
int gettok(char **);
int inarg(char c);
int procline(void);

/*program buffers and work pointers*/
static char inpbuf[MAXBUF], tokbuf[2*MAXBUF], *ptr = inpbuf, *tok = tokbuf, special [] = {' ', '\t', '&', ';', '\n', '\0'};
char *prompt = "Command> ";
pid_t shell;

int main()
{
	shell = getpid();
	job_list = malloc(sizeof(struct job));
	job_list->next = NULL;
	
	static struct sigaction act, act2, act3, act4;
	
	act.sa_handler = end_Process;
	act2.sa_handler = stop_Process;
	act3.sa_handler = job_remove_handle;
	act4.sa_handler = killTerminal;
	
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGTSTP, &act2, NULL);
	sigaction(SIGCHLD, &act3, NULL); 
	sigaction(SIGQUIT, &act4, NULL); //Allows for the termination of the small shell
	
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
						printJobList(job_list);
					}
					else if(strcmp("bg\0",arg[0]) == 0 | strcmp("fg\0",arg[0]) ==0)
					{
						if(arg[1] != NULL)
							if(strcmp("bg\0",arg[0]))
							{
								adjustJob(BACKGROUND, atoi(arg[1])); 
							}
							else if( strcmp("fg\0",arg[0])){
								adjustJob(FOREGROUND, atoi(arg[1])); 
							}
					}
					else if(strcmp("kill\0", arg[0]) == 0)
					{
						int pid = 0 - atoi(arg[1])	;
						kill(SIGINT, pid);
					}else		
						runcommand(arg, type);
				
				}
				
				if(toktype == EOL)
					return 0;
				
				narg = 0;
				break;
		}
	}
}

/* execute a command with optional wait */
int runcommand(char **cline, int where){
	pid_t pid;
	int status;
	struct job *new;

	switch(pid = fork()){
		case -1:
			perror("smallsh");
			return (-1);
		case 0:			
			execvp(*cline, cline);
			perror(*cline);
			//printf("fail");
			exit(1);
		default:
			new = malloc( sizeof(struct job) );
			
			if(where == FOREGROUND)
				NUMFG++;
			if(where == BACKGROUND)
				NUMBG++;
			
			new->jobID = NUMFG + NUMBG;
			new->pjob = malloc(sizeof(struct job));
			new->pjob->pid = pid;
			new->pjob->file = *cline;
			if(where == FOREGROUND)
				new->pjob->type = FOREGROUND;
			else
				new->pjob->type = BACKGROUND;
			
			new->pjob->running = RUNNING;
			addJob(new);
	}
	
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

void end_Process(int sig)
{
	int j ;

	if(NUMFG == 0)
	 {
		 printf("^C\n");
		system("./smallsh.exe");
	 }else {
		if((j= findFG()) == -1)
			kill(SIGKILL,j);
		else
		{
			printf("^C\n");
			system("./smallsh.exe");
		}
	 }
}

void stop_Process(int sig)
{
	pid_t pid;
	pid = findFG();
	printf("[Stopped] %d\n", pid);
	kill(SIGTSTP,pid);
}

void addJob(struct job* new)
{
	struct job *current;
	 
	if(job_list->next == NULL)
		job_list->next = new;
	else	 
		for( current = job_list->next; current != NULL ;current = current->next)
			if(current->next == NULL){
				current->next = new;
				break;
			}
}

pid_t findFG(){
	if(job_list->next == (struct job*)NULL)
	 {
		printf("No Jobs\n");
		//return -1;
	 }
	for(struct job* current = job_list->next; current != NULL; current = current->next)
		if(current->pjob->type == FOREGROUND)
			return current->pjob->pid;
}	

void printJobList(){
	
	 struct job *current;
	 
	 if(job_list->next == (struct job*)NULL)
	 {
		printf("JID PID TYPE RUNNING \n");
		return;
	 }
	 printf("JID PID TYPE RUNNING \n");
	 for( current = job_list->next; current != NULL ;current = current->next)
			if(current != NULL && current->pjob->type == BACKGROUND)
				printf("%d %3d %d %d\n", current->jobID, current->pjob->pid, current->pjob->type, current->pjob->running);
}


void job_remove_handle(int sig){
	struct job *current, *temp, *trail;
	int status;

	if(NUMBG <= 0 || NUMFG <= 0)
		return;
	
	for(current = job_list -> next, trail = job_list; current != NULL; current = current->next)
	{
		if(waitpid(current->pjob->pid,&status,0) == -1 ){
		
			if(current->pjob->running < STOPPED || current->pjob->running > RUNNING )
			{
				temp = current;
				trail->next =current->next;
				current = current->next;
			
				if(current->pjob->type == FOREGROUND)
					NUMFG--;
				if(current->pjob->type == BACKGROUND)
					NUMBG--;
				free(temp);
			}	
			break;
			
		}
		trail = current;
	}
}

void killTerminal(int sig)
{
	kill(SIGKILL, shell);
}

	
void adjustJob(int where,int jobID ){
	
	for(struct job *current = job_list->next; current != NULL; current = current->next)
	{
		if(current->jobID == jobID)
		{
			if(where == BACKGROUND)
			{
				current->pjob->running = RUNNING;
				kill(SIGCONT, current->pjob->pid);
			}
			else if(where == FOREGROUND)
			{
				
				current->pjob->running = RUNNING;
				current->pjob->type = where;
				kill(SIGCONT, current->pjob->pid);
			}
			
			break;
		}
	}
	
}
