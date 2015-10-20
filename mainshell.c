#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<signal.h>
#include<wait.h>
#include<fcntl.h>



void execute_external(char input[],char *arr[]);
void changedir(char *arr[]);
//void prompt();
void parse(char input[],char *arr[]);
void printhist();
void histn(int x);
void exec_histn(int x,char input[],char *arr[],pid_t main_pid,int i);
void execmain(char input[],char *arr[],pid_t main_pid,int i);
void child_handler(int signum);
void signal_handler(int signum);
void pipes_and_redirection(char input[]);
void only_redirection(char *input);


typedef struct node
{
	char name[1000];
	int status;
	int pid;
}node;

node hist[1000];
node back[1000];
node all[1000];

int parse_count=0;
int glob_count=1;
int back_count=0;
int all_count=0;
int flag=0;

char *MYHOME;

/****************************************************************/

void prompt()
{	
	char *username,hostname[1024],*cwd=NULL,finalpath[1000];
	strcpy (finalpath,"~");
	username=getenv("USER");
	//hostname=getenv("HOSTNAME");
	cwd = getcwd(cwd,1000);
	char *path=strstr(cwd,MYHOME);
	path=path+strlen(MYHOME);
	strcat(finalpath,path);
	gethostname(hostname,1023);
	printf("%s@%s:%s> ",username,hostname,finalpath);
	return;
}

/****************************************************************/

void exec_histvi(char x[],char input[],char *arr[],pid_t main_pid,int i)
{
	int pipeflag=0;
	int p,j=1,k,flag=0;
	if(glob_count-1>=0)
	{
		for(p=glob_count-1;p>=0;p--)
		{
			flag=0;
			j=0;
			int count=0;
			for(k=0;k<strlen(hist[p].name);)
			{
				if(hist[p].name[k]==x[j])
				{
					count++;
					j++;
					k++;
				}
				else
				{
					j=0;
					count=0;
					k++;
				}
				if(count==strlen(x))
				{
					flag=1;
					break;
				}
			}
			if(flag)
			{
				break;
			}
		}
	}
	else
	{

	}
	if(flag)
	{
		strcat(hist[p].name,"\0");
		char*temp=(char *)malloc(sizeof(char)*1000),*tempy;
		strcpy(temp,hist[p].name);
		//printf("%s\n",temp);
		tempy=temp;
		printf("%s\n", tempy);
		int pcount=0;
		char *array[1000];
		char *token=strtok(tempy," ");
		while(token!=NULL)
		{
			token[strlen(token)]='\0';
			array[pcount]=token;
			pcount++;
			token=strtok(NULL," ");
		}
		array[pcount++]=token;
		//execmain(tempy,arr,main_pid,i);
		execute_external(tempy,array);
		return;
	}
	return;
}

int main ()
{
	signal(SIGINT,SIG_IGN);
	signal(SIGINT,signal_handler);
	signal(SIGCHLD,SIG_IGN);
	signal(SIGCHLD,child_handler);
	signal(SIGTSTP,SIG_IGN);
	signal(SIGTSTP,signal_handler);
	signal(SIGQUIT,SIG_IGN);
	signal(SIGQUIT,signal_handler);

	MYHOME=getenv("PWD");
	pid_t main_pid=getpid();
	while(1)
	{
		prompt();
		int i=0,j=0,spaces=0,temp=0,pipeflag=0,redirectflag=0;
		char input[1000]={'\0'},ch,space_removed_input[1000]={0};
		char *arr[1000];
		scanf("%c",&ch);

		int sflag=0;
		if(ch=='\n')
			 sflag=1;

		while(ch!='\n')
		{
			if(ch=='&')
			{
				scanf("%c",&ch);
				flag=1;
				i--;
				continue;
			}
			input[i++]=ch;
			scanf("%c",&ch);
		}
		while(input[spaces]==' '|| input[spaces]=='\t')
			spaces++;
		while(input[temp]!='\0')
			space_removed_input[temp++]=input[spaces++];
		input[i]='\0';
		space_removed_input[temp]='\0';
		strcpy(input,space_removed_input);
		if((strcmp(hist[glob_count-1].name,input)!=0) && (sflag==0)) // ensuring that input is not same as previous command or null string.
			strcpy(hist[glob_count++].name,input);
		for( j=0;j<i;j++)
			if(input[j]=='|')
				pipeflag=1;
		if(pipeflag==1)
		{
			pipes_and_redirection(input);
			continue;
		}
		for(j=0;j<i;j++)
			if(input[j]=='<' || input[j]=='>')
				redirectflag=1;
		if(redirectflag==1 && pipeflag!=1)
		{
			only_redirection(input);
			continue;
		}
		char *input2=(char*)malloc(sizeof(strlen(input)+1));
		strcpy(input2,input);
		if(strcmp(input2,"\0"))
		{
			parse(input2,arr);
			execmain(input,arr,main_pid,i);
		}
		parse_count=0;
		flag=0;
	}
	glob_count=1;
	back_count=0;
	return 0;
}

void execmain(char input[],char *arr[],pid_t main_pid,int i)
{
	int counter=0;
	while(arr[counter]!=NULL)
	{
		if(!strcmp(arr[counter],"&"))	
		{
			arr[counter]=NULL;
			flag=1;
			break;
		}
		else
			counter++;
	}
	if (strcmp(arr[0],"exit")==0)
	{
		printf("Bye");
		exit(0);
	}
	else if(arr[0]==NULL)
	{
		prompt();
	}
	else if(strcmp(arr[0],"cd")==0)
	{
		changedir(arr);
	}
	else if(strcmp(arr[0],"history")==0 && counter==1)
	{
		printhist();
	}
	else if(strcmp(arr[0],"!history")==0)
	{
		printhist();
	}
	else if((strcmp(arr[0],"pid")==0) && parse_count-1==1)
		printf("./a.out process id: %d\n",main_pid);
	else if(strncmp(arr[0],"history",7)==0 && counter==2 && arr[1]!="|")
	{
		char *start=(char*)malloc(sizeof(strlen(start)+1));
		start=arr[0]+7;
		printf("%s",start);
		start[strlen(start)]='\0';
		histn(atoi(arr[1]));
	}
	else if(strncmp(arr[0],"history",7)==0 && counter>2 && strcmp(arr[1],"|")==0)
	{
		char *start=(char*)malloc(sizeof(strlen(start)+1));
		start=arr[0]+7;
		printf("%s",arr[3]);
		start[strlen(start)]='\0';
		histn(atoi(arr[1]));
	}
	else if(!strncmp(arr[0],"!",1) && ((arr[0][1]>='0' && arr[0][1]<='9') || arr[0][1]=='!'))
	{
		char *start=(char*)malloc(sizeof(strlen(start)+1));
		start=arr[0]+1;
		start[strlen(start)]='\0';
		if(arr[0][1]=='!')
		{
			exec_histn(glob_count-2,input,arr,main_pid,i);
		}
		else
			exec_histn(atoi(start),input,arr,main_pid,i);
	}
	else if(!strncmp(arr[0],"!",1) && (arr[0][1]>='a' && arr[0][1]<='z' ))
	{
		
		char *start=(char*)malloc(sizeof(strlen(start)+1));
		if(arr[0][1]=='v')
		{
			strcpy(start,"vi");
			start[strlen(start)]='\0';
			exec_histvi(start,input,arr,main_pid,i);
		}
		else if(arr[0][1]=='p')
		{
			char *array[100];
			array[0]="pwd";
			start[strlen(array[0])]='\0';
			execute_external(input,array);
		}
		else if(arr[0][1]=='l')
		{
			char *array[100];
			if(arr[1]=='\0')
				array[0]="ls";
			else
			{
				array[0]="ls";
				array[1]=arr[1];
				printf("%s\n", array[1]);
			}
			start[strlen(array[0])]='\0';
			execute_external(input,array);			
		}
		else if(arr[0][1]=='m')
		{
			char *array[100];
			array[0]="man";
			start[strlen(array[0])]='\0';
			execute_external(input,array);			
		}
		else if(arr[0][1]=='h')
		{
			printhist();			
		}
		else
			exec_histvi(start,input,arr,main_pid,i);
	}
	else if(strcmp(arr[0],"pid")==0 && strcmp(arr[1],"current")==0)
	{
		int i=0;
		printf("List of currently executing processes spawned from this shell:\n");
		for(i=0;i<back_count;i++)
			if(back[i].status==1)
				printf("command name: %s process id: %d\n",back[i].name,back[i].pid);
	}
	else if(strcmp(arr[0],"pid")==0 && strcmp(arr[1],"all")==0)
	{
		int i=0;
		printf("List of all processes spawned from this shell:\n");
		for(i=0;i<all_count;i++)
			printf("command name: %s process id: %d\n",all[i].name,all[i].pid);
	}
	
	else if((!strncmp(arr[0],"$",1) && arr[0][1]!='\0'))
	{
		char *pPath;
		if(strcmp(arr[0],"$PATH")==0)
		{
			pPath = getenv("PATH");
		}
		else if(strcmp(arr[0],"$HOME")==0)
		{
			pPath=getenv("HOME");
		}
		if (pPath!=NULL)
    			printf ("%s\n",pPath);
	}
	else if((!strcmp(arr[0],"echo") && arr[1][0]=='$') || (!strcmp(arr[0],"echo") && arr[1][0]=='"'))
	{
		char *pPath;
		int ll=strlen(arr[1]);
		if(arr[1][0]=='"')
		{
			strcpy(arr[1],(arr[1]+1));
			arr[1][ll-2]='\0';
		}
		if(strcmp(arr[1],"$PATH")==0)
		{
			pPath = getenv("PATH");
		}
		else if(strcmp(arr[1],"$HOME")==0)
		{
			pPath=getenv("HOME");
		}
		else if(strcmp(arr[1],"$PWD")==0)
		{
			pPath=getenv("PWD");
		}
		if(pPath!=NULL && arr[1][0]=='$')
		{
			if(strcmp(pPath,"echo")==0)
    			printf ("\n");
    		else
    			printf("%s\n", pPath);
		}
    	else
    			printf("%s\n",arr[1]);
	}
	else
	{
		if(flag==1)
			strcat(input,"&");
		execute_external(input,arr);
	}
	return;
}

/**************************************************************/

void parse(char input[],char *arr[])
{
	char *token=strtok(input," ");
	while(token!=NULL)
	{
		token[strlen(token)]='\0';
		arr[parse_count]=token;
		parse_count++;
		token=strtok(NULL," ");
	}
	arr[parse_count++]=token;
	return;
}

/***************************************************************/

void execute_external(char input[],char *arr[])
{
	pid_t p=fork();
	if(flag==0)
	{
		if(p<0)
		{
			perror("unable to fork");
			exit(1);
		}
		else if (p==0)
		{
			if(execvp(arr[0],arr)<0)
			{
				perror("command not found ... Is it in your path ?");
				_exit(0);
			}
		}
		else
		{
				all[all_count].pid=p;
				strcpy(all[all_count].name,input);
				all[all_count++].status=0;
				wait(NULL);
		}
	}
	else if (flag==1)
	{
		if(p<0)
		{
			perror("unable to fork");
			exit(1);
		}
		else if (p==0)
		{
			if(execvp(arr[0],arr)<0)
			{
				perror("maybe an in-built command");
				_exit(0);
			}
		}
		else
		{
			strcpy(back[back_count].name,input);
			back[back_count].pid=p;
			back[back_count++].status=1;

			all[all_count].pid=p;
			strcpy(all[all_count].name,input);
			all[all_count++].status=1;
		}
	}
}



/****************************************************************/

void changedir(char *arr[])
{
	char *cwd=NULL;
	char currpath[1000],tokens[100];
	if( arr[1]!=NULL ){
		strcpy(tokens,arr[1]);
		tokens[strlen(arr[1])]=0;
	}
	int ret,len=0;
	char pp[1000];
	if ((arr[1]==NULL) || (!strcmp(arr[1],"~")) || (!strcmp(arr[1],"~/")))
	{
		ret = chdir(MYHOME);
		if( ret!=0 )
			perror("chdir()");
		return;
	}
	if(strcmp(arr[1],"..")==0)
	{	
		cwd = getcwd(cwd,1000);
		len = strlen(cwd);
		strcpy(currpath,cwd);
		strcat(currpath,"/");
		int i=0;
		for(i=len-1;i>=0;i--)
		{
			if(cwd[i]=='/')
			{
				currpath[i]='\0';
				break;
			}
		}
		printf("%s\n",currpath);
		if (chdir(currpath) != 0)
			perror("chdir()");
		else
			return;
	}
	else if(strncmp(arr[1],"/home",5)==0)
	{
		strcpy(pp,"~");
		strcat(pp,arr[1]);
		ret = chdir(pp);
		if( ret!=0 )
			perror("chdir()");
		return;		
	}
	else
	{
		cwd = getcwd(cwd,1000);
		len = strlen(cwd);
		strcpy(currpath,cwd);
		strcat(currpath,"/");
		len = len + strlen(tokens) +1;
		if( tokens!=NULL )
			strcat( currpath, arr[1]);
		currpath[len]=0;
		printf("%s\n",currpath );
		if (chdir(currpath) != 0)
			perror("chdir()");
		else
			return;
	}
}

/****************************************************************/

void printhist()
{
	int i=0;
	for(i=1;i<glob_count;i++)
		printf("%d. %s\n",i,hist[i].name);
	return;
}

/****************************************************************/

void histn(int x)
{
	int i,j=1;
	if(glob_count-1>x)
		for(i=glob_count-x;i<=glob_count-1;i++)
			printf("%d. %s\n",i,hist[i].name);
	else
		printhist();
}

/****************************************************************/

void histnn(char *str)
{
	int i=0,j,k;
	
	for(i=1;i<glob_count;i++)
	{
		int flag=0;
		j=0;
		int count=0;
		for(k=0;k<strlen(hist[i].name);)
		{
			if(hist[i].name[k]==str[j])
			{
				count++;
				j++;
				k++;
			}
			else
			{
				j=0;
				count=0;
				k++;
			}
			if(count==strlen(str))
			{
				flag=1;
				break;
			}
		}
		if(flag)
		{
			printf("%d. %s\n",i,hist[i].name);
		}
	}
	return;	
}

/****************************************************************/

void exec_histn(int x,char input[],char *arr[],pid_t main_pid,int i)
{
	int pipeflag=0,j=0;
	parse_count=0;
	strcat(hist[x].name,"\0");
	printf("%s\n",hist[x].name);
	for(j=0;j<strlen(hist[x].name);j++)
	{
		if(hist[x].name[j]=='|')
		{
			pipeflag=1;
		}
	}
	if(pipeflag==1)
	{
		pipes_and_redirection(hist[x].name);
		return;
	}

	char*temp=(char *)malloc(sizeof(char)*1000),*tempy;
	strcpy(temp,hist[x].name);
	tempy=temp;
	parse(tempy,arr);			// parse would tear apart hist[x].name which is global, hence passing its copy to it .
	execmain(tempy,arr,main_pid,i);
	return;
}



/****************************************************************/

void child_handler(int signum)
{
	int p,i=0,count=0;
	p = waitpid(WAIT_ANY, NULL, WNOHANG);
	for(i=0;i<back_count;i++)
	{
		if(back[i].pid==p)
		{
			int j=0;
			char temp[100];
			for(j=0;back[i].name[j]!='&';j++)
			{
				temp[count++]=back[i].name[j];
			}
			temp[count]='\0';
			fflush(stdout);
			printf("\n%s %d exited normally\n",temp,back[i].pid);
			prompt();
			fflush(stdout);
			back[i].status=0;
		}
	}
	signal(SIGCHLD, child_handler);
	return;
}

/*************************************************************************/

void signal_handler(int signum)
{
	if(signum==2 || signum==20 || signum ==3)
	{
		fflush(stdout);
		printf("\n");
		prompt();
		fflush(stdout);
		signal(SIGINT, signal_handler);
		signal(SIGQUIT, signal_handler);
		signal(SIGTSTP,signal_handler);
	}
	return;
}

/*************************************************************************/


void pipes_and_redirection(char input[])
{
	char *temp = NULL, *pipeCommands[1000], *cmdArgs[1000],*newcmdArgs[1000];
	int newPipe[2], oldPipe[2], pipesCount, aCount=0, i, status,newaCount=0,in,out; //newPipe[0]=read; newPipe[1]=write
	pid_t pid;
	pipesCount = 0; 					// no of pipe commands to be executed, NOTE : not pipe count :P

	char *token=strtok(input,"|");
	while(token!=NULL)
	{
		token[strlen(token)]='\0';
		pipeCommands[pipesCount]=token;
		pipesCount++;
		token=strtok(NULL,"|");
	}
	pipeCommands[pipesCount++]=token;
	pipesCount--;						//pipeCount has been increased by more than one , hence decreasing
	if(strncmp(pipeCommands[0],"history",7)==0)
	{
		int pcount=0;
		char *array[1000];
		char *token=strtok(pipeCommands[1]," ");
		while(token!=NULL)
		{
			token[strlen(token)]='\0';
			array[pcount]=token;
			pcount++;
			token=strtok(NULL," ");
		}
		array[pcount++]=token;
		histnn(array[1]);		
		return;
	}
	int stdin_copy, stdout_copy;				// creating copy of stdin and stdout to restore later on 
	stdin_copy=dup(STDIN_FILENO);	
	stdout_copy=dup(STDOUT_FILENO);

	for(i = 0; i < pipesCount; i++) /* For each command */
	{
		int inflag=0,outflag=0,inindex=-1,outindex=-1,j; 
		for(j=0;j<strlen(pipeCommands[i]);j++)
		{
			if(pipeCommands[i][j]=='<')		// check if it has input redirection
			{
				inflag=1;
				inindex=i;
			}
			if(pipeCommands[i][j]=='>')		//check if it has output redirection
			{
				outflag=1;
				outindex=i;
			}
		}
		aCount = 0;                                   // argument count

		/* parsing in case of inflag */

		if(inflag==1)
		{

			token=strtok(pipeCommands[i],"<");	// parsing , tokenising along <
			while(token!=NULL)
			{
				token[strlen(token)]='\0';
				cmdArgs[aCount]=token;
				aCount++;
				token=strtok(NULL,"<");
			}
			cmdArgs[aCount++]=token;		//cmdArgs[1] is for sure a file name.		

			newaCount=0;

			token=strtok(cmdArgs[0]," ");		// tokenising cmdArgs[0] , which is a command and may have flags .
			while(token!=NULL)
			{
				token[strlen(token)]='\0';
				newcmdArgs[newaCount]=token;
				newaCount++;
				token=strtok(NULL," ");
			}
			newcmdArgs[newaCount++]=token;		// newcmdArgs has all the flags in it (along with the command name).

			in = open(cmdArgs[1], O_RDONLY , 0664);	// open the file with fd = in.
			if (in < 0) {
				perror(cmdArgs[1]);		// error if file doesn't exist.
				exit(1);
			}
				dup2(in, STDIN_FILENO);
			
		}

		/* parsing in case of outflag  */

		else if(outflag==1)
		{
			token=strtok(pipeCommands[i],">");
			while(token!=NULL)
			{
				token[strlen(token)]='\0';
				cmdArgs[aCount]=token;
				aCount++;
				token=strtok(NULL,">");
			}
			cmdArgs[aCount++]=token;


			cmdArgs[2]=NULL;
			newaCount=0;
			
			token=strtok(cmdArgs[0]," ");
			while(token!=NULL)
			{
				token[strlen(token)]='\0';
				newcmdArgs[newaCount]=token;
				newaCount++;
				token=strtok(NULL," ");
			}
			newcmdArgs[newaCount++]=token;

			out = open(cmdArgs[1], O_CREAT | O_WRONLY, S_IRWXU);
			if (out < 0) {
				perror(cmdArgs[1]);
				exit(1);
			}
					dup2(out, STDOUT_FILENO);
		}

		/* Parsing command & arguments in case of only pipes*/
		else
		{
			token=strtok(pipeCommands[i]," ");
			while(token!=NULL)
			{
				token[strlen(token)]='\0';
				cmdArgs[aCount]=token;
				aCount++;
				token=strtok(NULL," ");
			}
			cmdArgs[aCount++]=token;
		}

		/* If there still are commands to be executed */
		if(i < pipesCount-1) 
			pipe(newPipe); /* create a pipe */
		pid = fork();

		if(i>0 && i<pipesCount-1)	//for !first && !last command.
		{
			dup2(oldPipe[0], 0); //oldpipe[0] is the new stdin; ie writes happen at oldpipe[0]
			close(oldPipe[1]);
			close(oldPipe[0]);

		}
		else if(i==pipesCount-1)	//for last command.
		{
			dup2(oldPipe[0], 0); //oldpipe[0] is the new stdin; ie writes happen at oldpipe[0]
			close(oldPipe[1]);
			close(oldPipe[0]);

		}

		if(pid == 0)  /* Child */
		{
			if(i==0)
			{
				dup2(newPipe[1], 1); // stdout goes into newpipe[1]
				close(newPipe[0]);
				close(newPipe[1]);

			}

			/*********************has a fuck up here**********************/

			if(i>0 && i<pipesCount-1)
			{
				dup2(newPipe[1], 1); // stdout goes into newpipe[1]
				close(newPipe[0]);
				close(newPipe[1]);

			}
			if(!(inflag || outflag))		// normal execvp with cmdArgs[0] and cmdArgs as input.
			{
				int res = execvp(cmdArgs[0], cmdArgs);
				if (res == -1)
					printf("Error. Command not found: %s\n", cmdArgs[0]);
				exit(1);
			}
			if(outflag==1)				// execvp with cmdArgs[0] nad newcmdArgs as input. ( newcmdArgs has flags)
			{
					if (execvp(cmdArgs[0],newcmdArgs) < 0) {
						perror("execvp");
						exit(1);
					}
			}
			if(inflag==1)				// execvp with cmdArgs[0] and newcmdArgs as input.
			{
				if (execvp(cmdArgs[0],newcmdArgs) < 0) {
					perror("execvp");
					exit(1);
				}
			}
		} 
		else /* Father */
		{		
			waitpid(pid, &status, 0);			// wait for each child to die.

		/* to recover pids later on for use in pid commands */ 

			all[all_count].pid=pid;
			strcpy(all[all_count].name,cmdArgs[0]);
			all[all_count++].status=0;

		/* saving pipes to take input from these later on if needed .*/

			if(i < pipesCount-1) 
			{
				oldPipe[0] = newPipe[0];
				oldPipe[1] = newPipe[1];
			}
		}
	}

	/* close all shit and restore stdin and stdout */

	close(oldPipe[0]);
	close(newPipe[0]);
	close(oldPipe[1]);
	close(newPipe[1]);
	// restore stdin, stdout
	dup2(stdin_copy, 0);
	dup2(stdout_copy, 1);
	close(stdin_copy);
	close(stdout_copy);
}

void only_redirection(char *input)
{
	int stdin_copy,stdout_copy;
	stdin_copy=dup(STDIN_FILENO);
	stdout_copy=dup(STDOUT_FILENO);
	int outCount=0,inCount=0,argCount=0;
	int in,tempfd,i,j,spaces=0;
	char *token,*temp,*rightfile[1000],*cmdArgs[1000],*leftfile[1000];

	token=strtok(input,">");
	while(token!=NULL)
	{
		token[strlen(token)]='\0';
		rightfile[outCount]=token;
		outCount++;
		token=strtok(NULL,">");
	}
	rightfile[outCount++]=token;

	token=strtok(rightfile[0],"<");
	while(token!=NULL)
	{
		token[strlen(token)]='\0';
		leftfile[inCount]=token;
		inCount++;
		token=strtok(NULL,"<");
	}
	leftfile[inCount++]=token;

	token=strtok(leftfile[0]," ");
	while(token!=NULL)
	{
		token[strlen(token)]='\0';
		cmdArgs[argCount]=token;
		argCount++;
		token=strtok(NULL," ");
	}
	cmdArgs[argCount++]=token;

	if(inCount >2 && outCount==2)				// just input redirection is there.
	{
		pid_t pid;
		in = open(leftfile[1], O_RDONLY , 0664);	//open the file 
		if (in < 0) {
			perror(leftfile[1]);			//error if it doesn't exist
			exit(1);
		}
		pid = fork();
		if (!pid) {					// child
			dup2(in, 0);				// make input stream as "in" .
			if (execvp(cmdArgs[0],cmdArgs) < 0) {	//error if command doesn't exist.
				perror("execl");
				exit(1);
			}
		
		}
		else						//parent
			wait(NULL);
	}

	else if(outCount>2 && inCount==2)			// just output redirection is there.
	{
		pid_t pid;
		int out;
		out = open(rightfile[1], O_CREAT | O_WRONLY, S_IRWXU);
		if (out < 0) {
			perror(rightfile[1]);
			exit(1);
		}
		pid = fork();
		if (!pid) {					//child
			dup2(out, STDOUT_FILENO);		// now output is "out" instead of STDOUT_FILENO(stdout)
			if (execvp(cmdArgs[0],cmdArgs) < 0) {
				perror("execl");
				exit(1);
			}
		}
		else						//parent
			wait(NULL);
	}

	/* both input and output redirection is there */

	else						
	{
		pid_t pid;
		int out,in;
		out = open(rightfile[1], O_CREAT | O_WRONLY, S_IRWXU);
		in = open(leftfile[1], O_RDONLY , 0664);

		if (out < 0) {
			perror(rightfile[1]);
			exit(1);
		}
		if(in <0)
		{
			perror(leftfile[1]);
			exit(1);
		}
		pid = fork();
		if (!pid) {				//child
			dup2(in,STDIN_FILENO);		// in is the new stdin
			dup2(out, STDOUT_FILENO);	//out is the new stdout
			if (execvp(cmdArgs[0],cmdArgs) < 0) {		
				perror("execl");
				exit(1);
			}
		}
		else					//parent
			wait(NULL);
	}
	dup2(stdin_copy, 0);				//restoring stdout and stdin back , closing file descriptors.
	dup2(stdout_copy, 1);
	close(stdin_copy);
	close(stdout_copy);

}

