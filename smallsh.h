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

int numjob = 0;

struct process_node{
	pid_t pid;
	int type;
	int running;
	struct process_node *next;
};

struct job{
	int jobID;
	struct process_node *pjob;
	struct job *next;
}*job_list	;


int userin(char *);
int gettok(char **);
int inarg(char c);
int procline(void);

void end_Process();
void stop_Process();

void addJob(struct job*);
void job_remove_handle();

void printJobList();	
void setType(int, pid_t);

pid_t findFG();
