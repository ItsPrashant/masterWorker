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
#include <errno.h>





#define SEQUENTIAL 1
#define SELECT 2
#define POLL 3
#define EPOLL 4
#define BUFFER_LEN 100


//////////////////////////////////////////////////////////////////
//////////*********** All Global varibales ***********/////////////////
//////////////////////////////////////////////////////////////////
char worker[15];
char x[10];
int pipes[10000][2];
int maxWorkers,n;
int mechanism=0;
int availWorkers;
int maxFd=-1; 
int activeWorkers;
int nextN;
int keeper=0;
double totalSum=0;
/////////////////////////////////////////
fd_set readFds;

/////////////////////////////////////////

struct pollfd pfds[10000];

//////////////////////////////////////////

int epfd;
struct epoll_event event;


//////////////////////////////////////////////////////////////////
//////////*********** All function declarations ***********/////////////////
//////////////////////////////////////////////////////////////////

char receivedMsg[BUFFER_LEN];
float getFloatValue(char *str);
void createWorker(int i,int pipeI);
void parseArguments(int argc,char **argv);
double myAtof(char *str);

//////////////////////////////////////////////////////////////////
//////////*********** main() function ***********/////////////////
//////////////////////////////////////////////////////////////////
int main(int argc,char **argv)
{
	printf("Master\n");
	parseArguments(argc,argv);
	//printf("After parsing arguments are:\nx=%s\nn=%d\nmechainsm:%d\nmaxWokers:%d\n",x,n,mechanism,maxWorkers);
	FD_ZERO(&readFds);
	epfd=epoll_create(n);
	struct epoll_event events[n];
								
	if(n<=maxWorkers)
	{
		for(int i=1;i<=n;i++)
		{
			//printf("Creating worker %d\n",i);
			createWorker(i,i-1);			
		}
		switch(mechanism)
		{
			case SEQUENTIAL:
				for(int i=0;i<n;i++)
				{
					close(pipes[i][1]);
					read(pipes[i][0],receivedMsg,BUFFER_LEN);	
					float ff=getFloatValue(receivedMsg);
					printf("%d. Received from child :%f\n",++keeper,ff);
					close(pipes[i][0]);
					totalSum+=ff;	
				}

			break;

			case SELECT:
				
				//FD_ZERO(&readFds);
				//int maxFd=-1; 
				for(int i=0;i<n;i++)
				{
					close(pipes[i][1]);
					//if(maxFd<pipes[i][0])
					//maxFd=pipes[i][0];
					FD_SET(pipes[i][0],&readFds);
				}
				fd_set copy; 
				//printf("maxFd=%d \n",maxFd);
				availWorkers=n;
				copy=readFds;
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
								float ff=getFloatValue(receivedMsg);
								printf("Master received: %f \n",ff);
								totalSum+=ff;
								FD_CLR(pipes[i][0],&copy);
								availWorkers--;
							}
							//else
							//FD_SET(pipes[i][0],&readFds);
						}
					}
					else
					{
						printf("Time up\n");
					}
					//printf("availWorkers= %d\n",availWorkers);
					readFds=copy;
					/*
					for(int i=0;i<n;i++)
					{
						//close(pipes[i][1]);
						//if(maxFd<pipes[i][0])
						//maxFd=pipes[i][0];
						FD_SET(pipes[i][0],&readFds);
					}
					*/
				}
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
								float ff=getFloatValue(receivedMsg);
								printf("%d. Master received: %f\n",++keeper,ff);
								totalSum+=ff;
								pfds[i].fd=-1;
								availWorkers--;
							}
						}
						
					}
					
				}
			break;
			case EPOLL:
				
				if(epfd==-1)
					perror("epoll_create() failed\n");
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
						//printf("Something available\n");
						
						for(int i=0;i<waitingChild;i++)
						{
							
							read(events[i].data.fd,receivedMsg,BUFFER_LEN);
							float ff=getFloatValue(receivedMsg);
							printf("%d. Master received: %f\n",++keeper,ff);
							totalSum+=ff;
							epoll_ctl(epfd,EPOLL_CTL_DEL,events[i].data.fd,NULL);
							availWorkers--;
						}
					}						
				}
					

			break;
		}
	}
/////////////////////////////////////////////////////////////////////////////////////////////////
//////////*********** For n greater than maxWorkers declarations ***********/////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
	else
	{
		
		for(int i=1;i<=maxWorkers;i++)
		{
			createWorker(i,i-1);			
		}
		activeWorkers=maxWorkers;
		nextN=maxWorkers+1;
		//totReceivedData=0;
		int index;
		int totRecv=0;		
		
		switch(mechanism)
		{
			case SEQUENTIAL:
				index=0;
				while(totRecv<n)
				{
					read(pipes[index][0],receivedMsg,BUFFER_LEN);
							
					float ff=getFloatValue(receivedMsg);
					printf("%d. Master received: %f\n",++keeper,ff);
					totalSum+=ff;
					if(nextN<=n)
					{
						close(pipes[index][0]);
						close(pipes[index][1]);
						createWorker(nextN,index);
						nextN++;
					}
					index=(index+1)%maxWorkers;		
					totRecv++;
				}
			break;
			case SELECT:
				for(int i=0;i<maxWorkers;i++)
				{
					close(pipes[i][1]);
					//if(maxFd<pipes[i][0])
					//maxFd=pipes[i][0];
					FD_SET(pipes[i][0],&readFds);
				}
				fd_set copy; 
				//printf("maxFd=%d \n",maxFd);
				availWorkers=maxWorkers;
				copy=readFds;
				while(availWorkers!=0)
				{
//					printf("Available workers= %d and maxFd=%d \n",availWorkers,maxFd);
					int r=select(maxFd+1,&readFds,NULL,NULL,NULL);//Change max fd
					if(r<0)
					{
						perror("select() error\n");
						exit(-1);
					}
					if(r>0)
					{	

						for(int i=0;i<maxWorkers;i++)
						{
							if(FD_ISSET(pipes[i][0],&readFds))
							{
								int nChar=read(pipes[i][0],receivedMsg,BUFFER_LEN);
								float ff=getFloatValue(receivedMsg);
								printf("%d. Master received: %f \n",++keeper,ff);
								totalSum+=ff;
								FD_CLR(pipes[i][0],&copy);
								
								if(nextN<=n)
								{
									//close(pipes[i][0]);
									close(pipes[i][1]);
									createWorker(nextN,i);
									nextN++;
									FD_SET(pipes[i][0],&copy);
									//availWorkers++;
								}
								else
								availWorkers--;
								
							}
							//else
							//FD_SET(pipes[i][0],&readFds);
						}
					}
					else
					{
						printf("Time up\n");
					}
					//printf("availWorkers= %d\n",availWorkers);
					readFds=copy;
				
				}
			break;
			case POLL:
				
				for(int i=0;i<maxWorkers;i++)
				{
					close(pipes[i][1]);
					pfds[i].fd=pipes[i][0];
					pfds[i].events=POLLIN;
				}
				availWorkers=maxWorkers;
				while(availWorkers!=0)
				{
					int waitingChild;
					waitingChild=poll(pfds,maxWorkers,-1);
					if(waitingChild==-1)
					{
						perror("poll() error\n");
					}
					if(waitingChild>0)
					{	
						//printf("Something available\n");
						
						for(int i=0;i<maxWorkers;i++)
						{
							if(pfds[i].revents&POLLIN)
							{
								//keeper++;
								read(pfds[i].fd,receivedMsg,BUFFER_LEN);
								float ff=getFloatValue(receivedMsg);
								printf("%d. Master received: %f\n",++keeper,ff);
								totalSum+=ff;
								if(nextN<=n)
								{
									//close(pipes[i][0]);
									//close(pfds[i].fd);
									createWorker(nextN,nextN-1);
									close(pipes[nextN-1][1]);
									pfds[i].fd=pipes[nextN-1][0];
									pfds[i].events=POLLIN;
									nextN++;
									//availWorkers++;
								}
								else
								{
									//close(pfds[i].fd);
									pfds[i].fd=-1;
									availWorkers--;
								}
							}
						}
						
					}
					
				}
			break;
			case EPOLL:
			
				if(epfd==-1)
					perror("epoll_create() failed\n");
				for(int i=0;i<maxWorkers;i++)
				{
					
					event.data.fd = pipes[i][0]; // return the fd to us later 
					event.events = EPOLLIN;
					int check = epoll_ctl (epfd, EPOLL_CTL_ADD, pipes[i][0], &event);
					if (check<0)
						perror ("epoll_ctl");
				}
				int waitingChild;
				availWorkers=maxWorkers;
				while(availWorkers!=0)
				{
					waitingChild=epoll_wait(epfd,events,availWorkers,-1);
					if(waitingChild==-1)
					{
						perror("epoll_wait() error\n");
					}
					if(waitingChild>0)
					{	
						//printf("Something available\n");
						
						for(int i=0;i<waitingChild;i++)
						{
							
							read(events[i].data.fd,receivedMsg,BUFFER_LEN);
							float ff=getFloatValue(receivedMsg);
							printf("%d. Master received: %f\n",++keeper,ff);
							totalSum+=ff;
							epoll_ctl(epfd,EPOLL_CTL_DEL,events[i].data.fd,NULL);
							if(nextN<=n)
							{
								//close(pipes[i][0]);
								close(events[i].data.fd);
								createWorker(nextN,nextN-1);
								event.data.fd = pipes[nextN-1][0]; // return the fd to us later 
								event.events = EPOLLIN;
								int check = epoll_ctl (epfd, EPOLL_CTL_ADD, pipes[nextN-1][0], &event);
								if (check<0)
									perror ("epoll_ctl");
		
								nextN++;
								//availWorkers++;
							}
							else
							{
								availWorkers--;
							}
						}
					}						
				}
					

			break;
		}
	}
	printf("\nTotal sum of the series is: %f\n",totalSum);
}

double myAtof(char *str)
{
	char *end;
	double dd=strtod(str,&end);
	if ((errno == ERANGE ) || (errno != 0 && dd == 0)) {
        perror("strtod()");
        exit(EXIT_FAILURE);
    }

   if (end== str) {
	   fprintf(stderr, "No digits were found\n");
	   exit(EXIT_FAILURE);
   }
   return dd;
}

float getFloatValue(char *str)
{
	int ptr=0;
	char newStr[50];
	while(str[ptr++]!=' ');
	int temp=ptr;
	while(str[ptr]!='\0'&&((str[ptr]>='0'&&str[ptr]<='9')||str[ptr]=='.'))
	{
		newStr[ptr-temp]=str[ptr];
		ptr++;
	}
	newStr[ptr-temp]='\0';
	//printf("%s and strlen= %d\n",newStr,(int)strlen(newStr));
	float x=atof(newStr);
	return x;
}

void createWorker(int i,int pipeI)
{

	if(pipe(pipes[pipeI])==-1)
	{
		perror("Pipe error\n");
		exit(-1);
	}
	if(maxFd<pipes[pipeI][0])
		maxFd=pipes[pipeI][0];
	//FD_SET(pipes[i-1][0],&readFds);
	int check=fork();
	if(check==-1)
	perror("fork() failed\n");
	else
	if(check==0)
	{
		close(pipes[pipeI][0]);
		dup2(pipes[pipeI][1],1);
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
		printf("Usage:./master --worker_path [/path/to/worker] --num_workers [numberOfWorkers] --wait_mechanism [sequential/select/poll/epoll] -x [valueOfX] -n [valueOfN]");
		exit(1);
	}
	
	for(int i=1;i<=9;i=i+2)
	{
		//printf("i= %d\n",i);
		if(!strcmp("--worker_path",argv[i]))
		{
			
			strcpy(worker,argv[i+1]);
			/*
			if(execlp(worker,"worker","-x","1","-n","1",NULL)==-1)
			{
				printf("#### Pass valid worker path ######\n");
				exit(-1);
			}
			*/ 
			continue;
		}
		else
		if(!strcmp("--num_workers",argv[i]))
		{
			maxWorkers=myAtof(argv[i+1]);
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
			n=myAtof(argv[i+1]);
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
				printf("Usage:./master --worker_path [/path/to/worker] --num_workers [numberOfWorkers] --wait_mechanism [sequential/select/poll/epoll] -x [valueOfX] -n [valueOfN]");
				exit(-1);
	
			}
			continue;
		}
		else
		{
			printf("Usage:./master --worker_path [/path/to/worker] --num_workers [numberOfWorkers] --wait_mechanism [sequential/select/poll/epoll] -x [valueOfX] -n [valueOfN]");
			exit(-1);
	
		}
		
	}
	printf("--worker_path= %s --num_workers= %d --wait_mechanism %d -x %s -n %d\n",worker,maxWorkers,mechanism,x,n);
	
}
