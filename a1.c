#include <signal.h>
#include <stdlib.h> 
#include <stdio.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/wait.h> 
#include <signal.h> 
#include <strings.h>
#include <fcntl.h>

#define MAX_JOBS 20
//#define COMMAND_SIZE 20

//construct a job object with two attributes: jobPid and command type. 
struct job{
	pid_t jobPid;
	//char* command[COMMAND_SIZE];
}jobList[MAX_JOBS];

//global variable for the size of jobList.
int size = 0;
pid_t child_pid;

//add a new job into the joblist and update the size of the list after inserting. 
void addJob(char * args[], pid_t pid, int cnt, int bg){
	if(args[0]!=NULL && size < MAX_JOBS && bg==1){
		for(int i=0;i<cnt;i++){
			//jobList[size].command[i] = args[i];
			jobList[size].jobPid=pid;
		}
	}
	//increment the current size of jobList by one.
	size++;
}

//remove the job of index i in the jobList.c
void removeJob(int i){
	if(i>=0 && i<=size){
		//int j=0;
		while(i<=size){
			jobList[i].jobPid=jobList[i+1].jobPid;

			i++;
		}		
	}
	//deduct the current size of jobList by one. 
	size--;
}
	

//print all background jobs in the current jobList 
//format [Job index]    pid[jobPid] status  command
void printBakcgroundJobs(){
	for(int i=0;i<size;i++){
		//print job index and the corresponding job pid
		printf("\n[%d]          pid[%d]\n",i,jobList[i].jobPid); 
		/*
		for(int j=0;j<COMMAND_SIZE;j++){
			//if command is not null then print the job command. 
			if(jobList[i].command[j]==NULL){
			}	
			else
				printf("  %s ", jobList[i].command[j]);
		}
		printf("\n");
		*/
	}
}



//all build-in commands
int buildIn(char *args[], int cnt){
    //cd change directory 
    if(strcmp("cd",args[0])==0){
            //there should be two arguments when changing directory: command and the path.
            if(cnt ==2 &args[1]!= NULL){
                if(!chdir(args[1])){
    //              getPWD(); //print pwd after changing the directory. 
                    return 1;
                }
                else{
                    printf("Error, current working directory shall remain unchanged");
                }
            } else{ //user only types one or more than two string token -> raised an error
                printf("Require one directory path");
            }
        }
        //pwd print current working directory, call getPWD();
        else if(strcmp("pwd",args[0])==0){
            if(cnt == 1){
                char cwd[100];
                getcwd(cwd,100);
                printf("%s\n",cwd);   
                return 0;
            }
            else{ //if user typed more than just the command pwd -> raised an error 
                printf("Error");
            }
        }

        //exit 
        else if(strcmp("exit",args[0])==0){
            if(cnt == 1){
                exit(0); //exit the shell. 
            }
            else{  //if user typed more than just the command exit -> raised an error
                printf("Error when exiting the shell, you only need to type command exit.");
            }
        }

        //jobs -> list all jobs that are running in the background, call function printBackgroundJobs.
        else if(strcmp("jobs",args[0])==0){
            if(cnt == 1){
                printBakcgroundJobs(); //call function printBackgroundJobs to print all background jobs in the list
                return 0;
            }   
            else{//if user typed more than just the command job -> raised an error
                printf("Error when executing jobs command");
            }
            
        }
        //fg - bring the job of input job index to foreground.
        else if (strcmp("fg",args[0])==0){
            if (cnt == 2 && args[1] !=NULL){
                //convert user typed job index(string) to actual job index(integer) for later use. 
                int index = atoi(args[1]) ; 
                //if conevrted job index is not in our current joblist range then we print an error argument.
                //i.e the index is less than 0 or it exceeds the size of current jobList.
                printf("current joblist"); 
                if(index < 0 || index > size ){
                    printf("Invalid Job Index");
                }
                else{
                    //get the pid of job. 
                    int pid = jobList[index].jobPid;
                    int status; 
                    //wait for child to finish that job. 
                    //printf("doing fg now!!!!");
                    removeJob(index);
                    printBakcgroundJobs();
                    waitpid(pid,&status,0);
                    //By bringing the background job to foreground we need to remove the background job in jobList
                   return 0;

                }

            }else{
                //if user typed more than just the required command, fg and a job index number -> raised an error
                printf("Error when executing fg command, require one job ID ");
            }
            
        }

        else{ //redirection

            //if user typed other commands than our currently build one. then use execvp to execute the command. 
           //execvp(args[0],args);
           return 0;

        }
}

//a > b, redirect a to b. 
int redirection(char *args[], int cnt){

	int symbol=0;
	//find the index when ">" occurs to seperate the two commands. 
	while(strcmp(args[symbol],">")!=0){
		symbol++;
	}
	
		//symbol ">" not found. 
	if(symbol<=0)
		return 1;
	else{
		//close the first command by a NULL.
		args[symbol] = NULL;
		//args[split+1] is the begining of the second command. 
		char *output = args[symbol+1];

		//if second command is not provided, then print error condition. 
		if(strlen(output)<=0){
			printf("Error, require one output file");
			return 1;
		}

		//fork
		int pid = fork();
		if(pid == -1 ){
			perror("fork error");
			return(EXIT_FAILURE);
		}
		else if(pid == 0){ //child processs
			//if output does not exist, create one, else just write the contents to it
			int fd = open(output, O_RDWR|O_CREAT,0666);
			close(1); //close STDOUT
			dup(fd); //duplicate it to fd
			close(fd); //close fd

			execvp(args[0],args); //execute the first command. 
			printf("error has occured");
			return(EXIT_FAILURE);
		}

		else { 
			int status;
			waitpid(pid,&status, 0); //wait for child process to finish
			return 0;
		}
	}
}


//a | b, pipe
int piping(char * args[], int cnt){

	int symbol=0;
	//find the index when "|" occurs to seperate the two commands. 
	while(strcmp(args[symbol],"|")!=0){
		symbol++;
	}
	//symbol ">" not found. 
	if(symbol<=0){
		return 1;
	}

	else{
		int p[2]; 

	    if (pipe(p) == -1) { 
	        printf("Error occurred during pipe, abort");
	        exit(EXIT_FAILURE);
	    }

		pid_t pid1 = fork();
		//p1 
		if(pid1 == 0){
			// 0 reads 1 writes
			//child process p1
			dup2(p[1],1); //duplicate the content of STDIN to p[1];
			close(p[0]);
			close(p[1]);
			args[symbol] = NULL;
			execvp(args[0],args);
			return 1;
		}

		else{
			//parent process p2
			pid_t pid2 = fork();

			if(pid2 == 0){
				//child process p2
				dup2(p[0],0);
				close(p[0]);
				close(p[1]);
				//printf("args[split+1] is  %s", args[split+1]);
				execvp(args[symbol+1], &args[symbol+1]);
				return 1;
			}
			int status;
			close(p[0]);
			close(p[1]);
			waitpid(pid1, &status, 0); //wait for pid1 to finish
			waitpid(pid2, &status, 0);  // wait for pid2 to finish
			return 0;
		}
	}

}

int getcmd(char *prompt, char *args[], int *background)
{
	 int length, i = 0;
	 char *token, *loc;
	 char *line = NULL;
	 size_t linecap = 0;
	 printf("%s", prompt);
	 length = getline(&line, &linecap, stdin);
	 if (length <= 0) {
		 exit(-1);
 	 }
 // Check if background is specified..
	 if ((loc = index(line, '&')) != NULL) {
		 *background = 1;
		 *loc = ' ';
	 } else
		 *background = 0;
	 char *line2 = line;
	 while ((token = strsep(&line2, " \t\n")) != NULL) {
	 	for (int j = 0; j < strlen(token); j++)
	 		if (token[j] <= 32)
				token[j] = '\0';
		if (strlen(token) > 0)
			args[i++] = token;
	 }
	 args[i]=NULL;

	return i;
}



//STGSTP - Ctrl + Z : ignore, shell not react to it
static void sigtstp_handler(int signo)
{
	return;
	//sigignore(signo);
}


//SIGINT - Ctrl+ C :kill the process that is running with the shell
static void sigint_handler(int signo)
{
	if (child_pid > 0 ) {
    	kill(child_pid, SIGINT);
    	waitpid(child_pid,0,0);
    	exit(0);
    }

}


void freeCmd(char* args[],int cnt){
	for(int i=0;i<cnt;i++){
		if (args[i] != NULL)
			args[i]=NULL;
	}
}

int main(void) {


    if (signal(SIGTSTP, sigtstp_handler) == SIG_ERR){
        printf("\ncan't catch SIGTSTP\n");
        exit(EXIT_FAILURE);
    }


    if (signal(SIGINT, sigint_handler) == SIG_ERR){
        printf("\ncan't catch SIGINT\n");
        exit(EXIT_FAILURE);
    }


     char *args[20];
     int bg;
     while(1) {
         bg = 0;
         int cnt = getcmd("\n>> ", args, &bg);
 /* the steps can be..:
(1) fork a child process using fork()
(2) the child process will invoke execvp()
(3) if background is not specified, the parent will wait,
otherwise parent starts the next command... */

//build-in funcs
	if(cnt<0)
		exit(1);
	if(cnt==0 || args[0]==NULL)
		continue;

	if(strcmp("exit",args[0])==0 && cnt==1)
        exit(0); //exit the shell. 

	if(cnt<3){
		buildIn(args,cnt);
	}
	else if(strcmp(args[cnt-2],">")==0){
		redirection(args,cnt);
	}
	else{
		piping(args,cnt);	
	}
	child_pid = fork();

    if (child_pid == (pid_t) -1) {
        printf("Error occurred during fork, abort\n");
        exit(EXIT_FAILURE);

    }

    else if(child_pid == (pid_t) 0){
		//child process, invoke execvp
    	if(strcmp(args[0],"pwd")!=0 && strcmp(args[0],"cd")!=0 && strcmp(args[0],"jobs")!=0 && strcmp(args[0],"fg")!=0 && cnt<3){
    		execvp(args[0],args);
    	}
    	exit(0);

    }

    else{
        //parent process
        if(bg==0){
            //not run in background, the parent will wait,
//          printf("\nparent process, bg = 0, not run in background, wait \n");
            int status;
            waitpid(child_pid,&status,0);
        }
        else{ 
            //runing in background, add current job to jobList. !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            addJob(args, child_pid, cnt, bg);
            printf("added one job, current size is %d",size);
        }
    }
	}
	return 0;
}