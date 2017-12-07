#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/select.h>
#include <sys/time.h>
#include <poll.h>

#define SEQUENTIAL 1
#define SELECT 2
#define POLL 3
#define EPOLL 4
#define BUFFER_LEN 100

char worker[15];
char x[10];
int pipes[1024][2];
int maxWorkers,n;
int mechanism=0;
int availWorkers;\
int maxFd=-1; 
/////////////////////////////////////////
fd_set readFds;

/////////////////////////////////////////

struct pollfd pfds[11];

//////////////////////////////////////////

int epfd;
struct epoll_event event;

////////////////////////////////////////////
char receivedMsg[BUFFER_LEN];

void createWorker(int i);
float strToFloat(char *s);
void parseArguments(int argc,char **argv);

int main(int argc,char **argv)
{
	printf("Master\n");
	parseArguments(argc,argv);
	//printf("After parsing arguments are:\nx=%s\nn=%d\nmechainsm:%d\nmaxWokers:%d\n",x,n,mechanism,maxWorkers);
	FD_ZERO(&readFds);
	epfd=epoll_create(n);
	if(epfd==-1)
				perror("epoll_create() failed\n");
		
	struct epoll_event events[n];
								
	if(n<=maxWorkers)
	{
		for(int i=1;i<=n;i++)
		{
			printf("Creating worker %d\n",i);
			createWorker(i);			
		}
		/*
		switch(mechanism)
		{
			case SEQUENTIAL:
				for(int i=0;i<n;i++)
				{
					close(pipes[i][1]);
					read(pipes[i][0],receivedMsg,BUFFER_LEN);	
					printf("Received from child %d :%s",i,receivedMsg);
					close(pipes[i][0]);	
				}

			break;
		
			case SELECT:
		*/		/*
				FD_ZERO(&readFds);
				int maxFd=-1; 
				for(int i=0;i<n;i++)
				{
					close(pipes[i][1]);
					if(maxFd<pipes[i][0])
					maxFd=pipes[i][0];
					FD_SET(pipes[i][0],&readFds);
				}
				 
				printf("maxFd=%d \n",maxFd);
				availWorkers=n;
				while(availWorkers!=0)
				{
					int r=select(maxFd+1,&readFds,NULL,NULL,NULL);//Change max fd
					if(r<0)
					{
						perror("select() error\n");
					}
					if(r>0)
					{	

						for(int i=0;i<n;i++)
						{
							if(FD_ISSET(pipes[i][0],&readFds))
							{
								int nChar=read(pipes[i][0],receivedMsg,BUFFER_LEN);
								printf("Master received: %s from pipe[%d][0]= %d\n",receivedMsg,i,pipes[i][0]);
								FD_CLR(pipes[i][0],&readFds);
								availWorkers--;
							}
						}
					}
					else
					{
						printf("Time up\n");
					}
					printf("availWorkers= %d\n",availWorkers);
				}
				/*
			break;
			case POLL:
				for(int i=0;i<n;i++)
				{
					close(pipes[i][1]);
					pfds[i].fd=pipes[i][0];
					pfds[i].events=POLLIN;
				}
				availWorkers=n;
				while(availWorkers!=0)
				{
					int waitingChild;
					waitingChild=poll(pfds,n,-1);
					if(waitingChild==-1)
					{
						perror("select() error\n");
					}
					if(waitingChild>0)
					{	
						//printf("Something available\n");
						
						for(int i=0;i<n;i++)
						{
							if(pfds[i].revents&POLLIN)
							{
								read(pfds[i].fd,receivedMsg,BUFFER_LEN);
								printf("Master received: %s\n",receivedMsg);
								pfds[i].fd=-1;
								availWorkers--;
							}
						}
						
					}
					
				}
			break;
			case EPOLL:
			*/	
				for(int i=0;i<n;i++)
				{
					event.data.fd = pipes[i][0]; // return the fd to us later 
					event.events = EPOLLIN;
					int check = epoll_ctl (epfd, EPOLL_CTL_ADD, pipes[i][0], &event);
					if (check<0)
						perror ("epoll_ctl");
				}
				int waitingChild;
				availWorkers=n;
				while(availWorkers!=0)
				{
					waitingChild=epoll_wait(epfd,events,availWorkers,-1);
					if(waitingChild==-1)
					{
						perror("epoll_wait() error\n");
					}
					if(waitingChild>0)
					{	
						printf("Something available\n");
						
						for(int i=0;i<waitingChild;i++)
						{
							
							read(events[i].data.fd,receivedMsg,BUFFER_LEN);
							printf("Master received: %s\n",receivedMsg);
							epoll_ctl(epfd,EPOLL_CTL_DEL,events[i].data.fd,NULL);
							availWorkers--;
						}
					}						
				}
					
			/*
			break;
		}
	}
	else
	{
		for(int i=1;i<=maxWorkers;i++)
		{
			createWorker(i);			
		}
		switch(mechanism)
		{
			case SEQUENTIAL:
			break;
			case SELECT:
			break;
			case POLL:
			break;
			case EPOLL:
			break;
		}
	}
	*/
	}
}





void createWorker(int i)
{
	if(pipe(pipes[i-1])==-1)
	{
		perror("Pipe error\n");
		exit(-1);
	}
	if(maxFd<pipes[i-1][0])
		maxFd=pipes[i-1][0];
	FD_SET(pipes[i-1][0],&readFds);
	int check=fork();
	if(check==-1)
	perror("fork() failed\n");
	else
	if(check==0)
	{
		
		close(pipes[i-1][0]);
		dup2(pipes[i-1][1],1);
		char nn[10];
		sprintf(nn,"%d",i);
		if(execlp(worker,"worker","-x",(const char*)x,"-n",(const char*)nn,NULL)==-1)
		{
			printf("execlp() failed\n");
		}

	}
}
void parseArguments(int argc,char **argv)
{
		if(argc!=11)
	{
		printf("Usage:./master --worker_path path/to/worker --num_workers numberOfWorkers --wait_mechanism sequential/select/poll/epoll -x valueOfX -n valueOfN");
		exit(1);
	}
	
	for(int i=1;i<=9;i=i+2)
	{
		printf("i= %d\n",i);
		if(!strcmp("--worker_path",argv[i]))
		{
			
			strcpy(worker,argv[i+1]);
			
			continue;
		}
		else
		if(!strcmp("--num_workers",argv[i]))
		{
			maxWorkers=strToFloat(argv[i+1]);
			continue;
		}
		else
		if(!strcmp("-x",argv[i]))
		{
			strcpy(x,argv[i+1]);
			continue;
		}
		else
		if(!strcmp("-n",argv[i]))
		{
			n=strToFloat(argv[i+1]);
			continue;
		}
		else
		if(!strcmp("--wait_mechanism",argv[i]))
		{
			if(!strcmp("sequential",argv[i+1]))
			mechanism=SEQUENTIAL;
			else
			if(!strcmp("select",argv[i+1]))
			mechanism=SELECT;
			else
			if(!strcmp("poll",argv[i+1]))
			mechanism=POLL;
			else
			if(!strcmp("epoll",argv[i+1]))
			mechanism=EPOLL;
			else
			{
				printf("Error in parsing mechanism\n");
				exit(1);
			}
			continue;
		}
		
	}
	printf("--worker_path= %s --num_worker= %d --wait_mechanism %d -x %s -n %d",worker,maxWorkers,mechanism,x,n);
	
}

float strToFloat(char *s)
{
	int len=strlen(s);
	
	float t=0;
	int i;
	for(i=0;i<len&&s[i]<='9'&&s[i]>='0';i++)
	{
		//printf("%c ",s[i]);
		if(s[i]=='.')
		t=t/pow(10,len-1-i);
		else
		t+=s[i]-'0';
		t=t*10;
	}
	if(i!=len)
	{
		printf("error in converting string to float\n");\
		exit(1);
	}
	//printf("\n");
	t=t/10;
	
	return t;
}
