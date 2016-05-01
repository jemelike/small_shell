/* smallsh.h -- defs for smallsh command processor */

#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

#define EOL 		1		/* end of line */
#define ARG 		2 		/* normal arguments */
#define AMPERSAND 	3
#define SEMICOLON 	4
#define PIPE		5

#define MAXARG 		512		/* max. no. command args */
#define MAXBUF 		512		/* max. length input line */

#define FOREGROUND 	0
#define BACKGROUND 	1

#define STOPPED 0
#define RUNNING 1 

//int NUMJOBS;
int NUMBG = 0;
int NUMFG = 0;

struct process_node{
	pid_t pid;
	int type;
	int running;
	char *file;
	struct process_node *next;
};

struct job{
	int jobID;
	struct process_node *pjob;
	struct job *next;
}*job_list	;


int runcommand(char **cline, int where);
int userin(char *);
int gettok(char **);
int inarg(char c);
int procline(void);

void end_Process(int sig);
void stop_Process(int sig);
void killTerminal(int sig);
void addJob(struct job*);
void job_remove_handle(int val);

void printJobList();	
void adjustJob(int,int);

pid_t findFG();
