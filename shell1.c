#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<signal.h>
#include<wait.h>
#include<fcntl.h>

#define sigchild(sigtype) signal(SIGCHLD, sigtype);
#define sighandler(sigtype) signal(SIGTSTP, sigtype);
#define sigintte(sigtype)   signal(SIGINT, sigtype);
#define strc(x,y)  strcpy(x,y)
#define strm(x,y)  strcmp(x,y)
#define stra(x,y)  strcat(x,y)
#define strt(x,y)  strtok(x,y)
#define strl(x)    strlen(x)
#define erro(x)      perror(x);

void exec_histn(int x,char input[],char *arr[],pid_t main_pid,int i);
void pipes_and_redirection(char input[]);
void only_redirection(char *input);

struct node
{
	char name[1000];
	int status;
	int pid;
};

int parse_count=0;
int glob_count=1;
int back_count=0;

struct node hist[1000];
struct node back[1000];
struct node all[1000];


int all_count=0;
int flag=0;

char *MYHOME;
//excute external commands
void execute_external(char input[],char *arr[])
{
	pid_t p=fork();
	if(flag==1)
	{
		if(p>0)
		{
			strc(back[back_count].name,input);
			back[back_count].pid=p;
			back[back_count].status=1;
			back_count++;

			all[all_count].pid=p;
			strc(all[all_count].name,input);
			all[all_count].status=1;
			all_count++;	
		}
		else if (p==0)
		{
			if(execvp(arr[0],arr)<0)
			{
				erro("maybe an in-built command");
				_exit(0);
			}
		}
		else
		{
			erro("unable to fork");
			exit(1);
		}	
	}
	else if (flag==0)
	{
		if(p>0)
		{
				all[all_count].pid=p;
				strc(all[all_count].name,input);
				all[all_count].status=0;
				all_count++;
				wait(NULL);
		}
		else if (p==0)
		{
			if(execvp(arr[0],arr)<0)
			{
				erro("command not found ... Is it in your path ?");
				_exit(0);
			}
		}
		else 
		{
				erro("unable to fork");
				exit(1);
		}
	}
}
//changing the directory
void changedir(char *arr[])
{
	char tokens[100];
	if( arr[1]!=NULL )
	{
		strc(tokens,arr[1]);
		int len=strl(arr[1]);
		tokens[len]=0;
	}
	int ret;
	if ((arr[1]==NULL) || (!strm(arr[1],"~")) || (!strm(arr[1],"~/")))
	{
		ret = chdir(MYHOME);
		if( ret!=0 )
			erro("chdir()");
		return;
	}
	char *cwd=NULL;
	cwd = getcwd(cwd,1000);

	int len=0;
	len = strl(cwd);

	char currpath[1000];
	strc(currpath,cwd);
	stra(currpath,"/");
	len = len + strl(tokens) +1;
	if( tokens!=NULL )
		stra( currpath, arr[1]);
	currpath[len]=0;
	if (chdir(currpath) == 0)
		return;
	else
	    erro("chdir()");
}
//prompt the shell
void prompt()
{	
	char *username,hostname[1024],*cwd=NULL,finalpath[1000];
	strc(finalpath,"~");
	cwd = getcwd(cwd,1000);
	char *path=strstr(cwd,MYHOME);
	path=path+strl(MYHOME);
	stra(finalpath,path);
	username=getenv("USER");
	gethostname(hostname,1023);
	printf("%s@%s:%s> ",username,hostname,finalpath);
	return;
}
//parsing input
void parse(char input[],char *arr[])
{
	char *token=strt(input," ");
	while(token!=NULL)
	{
		int len=strl(token);
		token[len]='\0';
		arr[parse_count++]=token;
		token=strtok(NULL," ");
	}
	arr[parse_count]=token;
	parse_count++;
}
//printing history
void printhist()
{
	int i=0;
	i=1;
	while(i<glob_count)
	{
		printf("%d. %s\n",i,hist[i].name);
		i++;
	}
}
void histn(int x)
{
	int i,j=1;
	if(glob_count-1<=x)
	{
		printhist();
	}
	else if(glob_count-1>x)
	{
		i=glob_count-x;
		while(i<=glob_count-1)
		{
			printf("%d. %s\n",j++,hist[i].name);
			i++;		
		}
	}	
}
//execute main
void execmain(char input[],char *arr[],pid_t main_pid,int i)
{
	int counter=0;
	while(arr[counter]!=NULL)
	{
		if(!strm(arr[counter],"&"))	
		{
			arr[counter]=NULL;
			flag=1;
			break;
		}
		else
			counter++;
	}
	if (strm(arr[0],"exit")==0)
	{
		exit(0);
	}
	else if(arr[0]==NULL)
	{
		prompt();
	}
	else if(!strm(arr[0],"cd"))
	{
		changedir(arr);
	}
	else if(!strm(arr[0],"history"))
	{
		printhist();
	}
	else if(!(strm(arr[0],"pid")) && parse_count-1==1)
	{
		printf("./a.out process id: %d\n",main_pid);
	}
	else if(!strncmp(arr[0],"history",4) && arr[0][4]!='\0')
	{
		char *start=(char*)malloc(sizeof(strlen(start)+1));
		start=arr[0]+4;
		start[strl(start)]='\0';
		histn(atoi(start));
	}
	else if(!strncmp(arr[0],"!history",5))
	{
		char *start=(char*)malloc(sizeof(strlen(start)+1));
		start=arr[0]+5;
		start[strl(start)]='\0';
		exec_histn(atoi(start),input,arr,main_pid,i);
	}
	else if(!strm(arr[0],"pid") && !strm(arr[1],"current"))
	{
		int i=0;
		printf("List of currently executing processes spawned from this shell:\n");
		while(i<back_count)
		{
			if(back[i].status==1)
				printf("command name: %s process id: %d\n",back[i].name,back[i].pid);
			i++;		
		}
	}
	else if(!strm(arr[0],"pid") && !strm(arr[1],"all"))
	{
		int i=0;
		printf("List of all processes spawned from this shell:\n");
		while(i<all_count)
		{
			printf("command name: %s process id: %d\n",all[i].name,all[i].pid);
			i++;		
		}
	}	
	else
	{
		if(flag==1)
			stra(input,"&");
		execute_external(input,arr);
	}
	return;
}
//child signal handler
void child_handler(int signum)
{
	int p,i,count=0;
	p = waitpid(WAIT_ANY, NULL, WNOHANG);
	i=0;
	while(i<back_count)
	{
		if(back[i].pid==p)
		{
			int j=0;
			char temp[100];
			j=0;
			while(back[i].name[j]!='&')			
			{
				temp[count]=back[i].name[j];
				count++;
				j++;
			}
			temp[count]='\0';
			fflush(stdout);
			printf("\n%s %d exited normally\n",temp,back[i].pid);
			prompt();
			fflush(stdout);
			back[i].status=0;
		}
		i++;
	}
	sigchild(child_handler);
	return;
}
//signal handler
void signal_handler(int signum)
{
	if(signum==2 || signum==20 || signum ==3)
	{
		fflush(stdout);
		printf("\n");
		prompt();
		fflush(stdout);
		sigintte(signal_handler)
		signal(SIGQUIT, signal_handler);
		signal(SIGTSTP,signal_handler);
	}
	return;
}
//main function
int main ()
{
	sigintte(SIG_IGN)
	sigintte(signal_handler)
	sigchild(SIG_IGN)
	sigchild(child_handler);
	sighandler(SIG_IGN)
	sighandler(signal_handler);
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
		strc(input,space_removed_input);
		// ensuring that input is not same as previous command or null string.
		if((strm(hist[glob_count-1].name,input)!=0) && (sflag==0)) 
			strc(hist[glob_count++].name,input);
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
		strc(input2,input);
		if(strm(input2,"\0"))
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
//executing the history function
void exec_histn(int x,char input[],char *arr[],pid_t main_pid,int i)
{
	int pipeflag=0,j=0;
	parse_count=0;
	stra(hist[x].name,"\0");
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
	strc(temp,hist[x].name);
	tempy=temp;
	// parse would tear apart hist[x].name which is global, hence passing its copy to it .
	parse(tempy,arr);			
	execmain(tempy,arr,main_pid,i);
	return;
}

void pipes_and_redirection(char input[])
{	
	pid_t pid;	
	//newPipe[0]=read; newPipe[1]=write
	int newPipe[2], oldPipe[2], pipesCount, aCount=0, i, status,newaCount=0,in,out; 
	// no of pipe commands to be executed, NOTE : not pipe count :P
	pipesCount = 0; 					
	char *temp = NULL, *pipeCommands[1000], *cmdArgs[1000],*newcmdArgs[1000];
	char *token=strt(input,"|");
	while(token!=NULL)
	{
		token[strlen(token)]='\0';
		pipeCommands[pipesCount]=token;
		pipesCount++;
		token=strt(NULL,"|");
	}
	pipeCommands[pipesCount++]=token;
	//pipeCount has been increased by more than one , hence decreasing
	pipesCount--;						
	// creating copy of stdin and stdout to restore later on
	int stdin_copy, stdout_copy;				 
	stdin_copy=dup(STDIN_FILENO);	
	stdout_copy=dup(STDOUT_FILENO);
	/* For each command */
	for(i = 0; i < pipesCount; i++) 
	{
		int inflag=0,outflag=0,inindex=-1,outindex=-1,j; 
		for(j=0;j<strlen(pipeCommands[i]);j++)
		{
			// check if it has input redirection
			if(pipeCommands[i][j]=='<')		
			{
				inflag=1;
				inindex=i;
			}
			//check if it has output redirection
			if(pipeCommands[i][j]=='>')		
			{
				outflag=1;
				outindex=i;
			}
		}
		// argument count
		aCount = 0;                                   
		/* parsing in case of inflag */
		if(inflag==1)
		{
			// parsing , tokenising along <
			token=strt(pipeCommands[i],"<");	
			while(token!=NULL)
			{
				token[strlen(token)]='\0';
				cmdArgs[aCount]=token;
				aCount++;
				token=strt(NULL,"<");
			}
			//cmdArgs[1] is for sure a file name.
			cmdArgs[aCount++]=token;				
			newaCount=0;
			// tokenising cmdArgs[0] , which is a command and may have flags .
			token=strtok(cmdArgs[0]," ");		
			while(token!=NULL)
			{
				token[strlen(token)]='\0';
				newcmdArgs[newaCount]=token;
				newaCount++;
				token=strt(NULL," ");
			}
			// newcmdArgs has all the flags in it (along with the command name).
			newcmdArgs[newaCount++]=token;		
			// open the file with fd = in.
			in = open(cmdArgs[1], O_RDONLY , 0664);	
			if (in < 0) {
				// error if file doesn't exist.
				perror(cmdArgs[1]);		
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
				token=strt(NULL,">");
			}
			cmdArgs[aCount++]=token;
			cmdArgs[2]=NULL;
			newaCount=0;			
			token=strt(cmdArgs[0]," ");
			while(token!=NULL)
			{
				token[strlen(token)]='\0';
				newcmdArgs[newaCount]=token;
				newaCount++;
				token=strt(NULL," ");
			}
			newcmdArgs[newaCount++]=token;
			out = open(cmdArgs[1], O_CREAT | O_WRONLY, S_IRWXU);
			if (out < 0) {
				erro(cmdArgs[1]);
				exit(1);
			}
					dup2(out, STDOUT_FILENO);
		}
		/* Parsing command & arguments in case of only pipes*/
		else
		{
			token=strt(pipeCommands[i]," ");
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
			/* create a pipe */
			pipe(newPipe); 
		pid = fork();
		//for !first && !last command.
		if(i>0 && i<pipesCount-1)	
		{
			//oldpipe[0] is the new stdin; ie writes happen at oldpipe[0]
			dup2(oldPipe[0], 0); 
			close(oldPipe[1]);
			close(oldPipe[0]);

		}
		//for last command.
		else if(i==pipesCount-1)	
		{
			//oldpipe[0] is the new stdin; ie writes happen at oldpipe[0]
			dup2(oldPipe[0], 0); 
			close(oldPipe[1]);
			close(oldPipe[0]);
		}
		/* Child */
		if(pid == 0)  
		{
			// stdout goes into newpipe[1]
			if(i==0)
			{
				dup2(newPipe[1], 1); 
				close(newPipe[0]);
				close(newPipe[1]);

			}
			// stdout goes into newpipe[1]
			if(i>0 && i<pipesCount-1)
			{
				dup2(newPipe[1], 1); 
				close(newPipe[0]);
				close(newPipe[1]);

			}
			// normal execvp with cmdArgs[0] and cmdArgs as input.
			if(!(inflag || outflag))		
			{
				int res = execvp(cmdArgs[0], cmdArgs);
				if (res == -1)
					printf("Error. Command not found: %s\n", cmdArgs[0]);
				exit(1);
			}
			// execvp with cmdArgs[0] nad newcmdArgs as input. ( newcmdArgs has flags)
			if(outflag==1)				
			{
					if (execvp(cmdArgs[0],newcmdArgs) >= 0) {
						erro("execvp");
						exit(1);
					}
					else if (execvp(cmdArgs[0],newcmdArgs) < 0) {
						erro("execvp");
						exit(1);
					}
			}
			// execvp with cmdArgs[0] and newcmdArgs as input.
			if(inflag==1)				
			{
				if (execvp(cmdArgs[0],newcmdArgs) >= 0) {
				}
				else if (execvp(cmdArgs[0],newcmdArgs) < 0) {
					erro("execvp");
					exit(1);
				}
			}
		} 
		/* Father */
		else 
		{	
			// wait for each child to die.	
			waitpid(pid, &status, 0);			
			/* to recover pids later on for use in pid commands */ 
			all[all_count].pid=pid;
			strc(all[all_count].name,cmdArgs[0]);
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
	int stdin_copy;
	char *rightfile[1000],*cmdArgs[1000];
	stdin_copy=dup(STDIN_FILENO);
	int stdout_copy;
	stdout_copy=dup(STDOUT_FILENO);
	int in,tempfd,i,j,spaces=0;
	char *token,*temp,*leftfile[1000];
	int outCount=0;
	token=strt(input,">");
	while(token!=NULL)
	{
		int len=strl(token);
		token[len]='\0';
		rightfile[outCount++]=token;
		token=strt(NULL,">");
	}
	rightfile[outCount]=token;
	outCount++;

	token=strt(rightfile[0],"<");
	int inCount=0;
	while(token!=NULL)
	{
		token[strlen(token)]='\0';
		leftfile[inCount++]=token;
		token=strt(NULL,"<");
	}
	leftfile[inCount]=token;
	inCount++;

	token=strt(leftfile[0]," ");
	int argCount=0;
	while(token!=NULL)
	{
		token[strlen(token)]='\0';
		cmdArgs[argCount++]=token;
		token=strt(NULL," ");
	}
	cmdArgs[argCount]=token;
	argCount++;

	// just input redirection is there.
	if(inCount >2 && outCount==2)				
	{
		pid_t pid;
		//open the file
		in = open(leftfile[1], O_RDONLY , 0664);	 
		if (in >= 0) {
			//do nothing
		}
		else if (in < 0) {
			//error if it doesn't exist
			erro(leftfile[1]);			
			exit(1);
		}
		pid = fork();
		// child
		if (pid==0) {
			// make input stream as "in" .					
			dup2(in, 0);		
			//error if command doesn't exist.		
			if (execvp(cmdArgs[0],cmdArgs) >= 0) {	
				//do nothing
			}
			else if (execvp(cmdArgs[0],cmdArgs) < 0) {	
				erro("execlp");
				exit(1);
			}		
		}
		//parent
		else						
			wait(NULL);
	}
	// just output redirection is there.
	else if(outCount>2 && inCount==2)			
	{
		pid_t pid;
		int out;
		out = open(rightfile[1], O_CREAT | O_WRONLY, S_IRWXU);
		if (out < 0) {
			perror(rightfile[1]);
			exit(1);
		}
		pid = fork();
		//child
		if (pid==0) {	
			// now output is "out" instead of STDOUT_FILENO(stdout)				
			dup2(out, STDOUT_FILENO);		
			if (execvp(cmdArgs[0],cmdArgs) < 0) {
				erro("execlp");
				exit(1);
			}
		}
		//parent
		else						
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
			erro(rightfile[1]);
			exit(1);
		}
		if(in <0)
		{
			erro(leftfile[1]);
			exit(1);
		}
		pid = fork();
		//child
		if (pid==0) {	
			// in is the new stdin			
			dup2(in,STDIN_FILENO);
			//out is the new stdout		
			dup2(out, STDOUT_FILENO);	
			if (execvp(cmdArgs[0],cmdArgs) >= 0) {		
				//do nothing
			}
			else if (execvp(cmdArgs[0],cmdArgs) < 0) {		
				erro("execlp");
				exit(1);
			}
		}
		else	
			//parent				
			wait(NULL);
	}
	//restoring stdout and stdin back , closing file descriptors.
	dup2(stdin_copy, 0);				
	dup2(stdout_copy, 1);
	close(stdin_copy);
	close(stdout_copy);
}


