#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include <sys/types.h>
#include <sys/epoll.h>

char *intToStr(int i,char *);
char worker[]="./worker";
int childAlive[5]={0};

int main()
{
	int noOfWorkers=3,bufferLen=100;
	int p[noOfWorkers][2],availWorkers=0;
	char buffer[100];
	int epfd;
	struct epoll_event event,events[noOfWorkers];
	char s1[10],s2[10];		
	int check;
	
	
	epfd=epoll_create(noOfWorkers);
	if(epfd==-1)
	perror("epoll_create() failed\n");
	
	printf("I am Master.\n");
	
	for(int i=1;i<=noOfWorkers;i++)
	{
		const char *chx=(const char*)intToStr(i,s1);
		const char *chn=(const char*)intToStr(i,s2);

		if(pipe(p[i-1])==-1)
		{
			perror("Pipe error\n");
			exit(-1);
		}
		event.data.fd = p[i-1][0]; // return the fd to us later 
		event.events = EPOLLIN;
		check = epoll_ctl (epfd, EPOLL_CTL_ADD, p[i-1][0], &event);
		if (check<0)
        perror ("epoll_ctl");
			
		check=fork();
		if(check==-1)
		perror("fork() failed\n");
		else
		if(check==0)
		{
			//printf("\nChild no.%d",i);
			close(p[i-1][0]);
			//printf("\nPipe value b: %d ",p[i-1][1]);
			dup2(p[i-1][1],1);
			//printf("\nPipe value a: %d ",p[i-1][1]);
			//printf("Hello dady. I am child no. %d",i);
			//close(p[i-1][1]);
			//return 1;
			
			//printf("Child no. %d",);
			
			
			if(execlp(worker,"worker","-x",chx,"-n",chn,NULL)==-1)
			{
				printf("execlp() failed\n");
			}
			
			
		}
		availWorkers++;
		
	}
	while(availWorkers!=0)
	{
			//printf("Avail workers:%d\n",availWorkers);
			//printf("waiting for poll\n");
			
			//printf("Returned by select(): %d\n",r);
			int waitingChild;
			waitingChild=epoll_wait(epfd,events,noOfWorkers,-1);
			if(waitingChild==-1)
			{
				perror("select() error\n");
			}
			if(waitingChild>0)
			{	
				//printf("Something available\n");
				
				for(int i=0;i<waitingChild;i++)
				{
					
					read(events[i].data.fd,buffer,bufferLen);
					printf("Master received: %s\n",buffer);
					availWorkers--;
				}
			}
				
			
			
		}
		
		
	
	
	printf("Master exits\n");

}
int max(int a1[],int b1[],int n)
{
	for(int i=0;i<n;i++)
	{
		if(b1[i]==1)
		{
			;
		}
	}
}
char *intToStr(int i,char *s)
{
	//char s[10];
	int j=0;
	while(i>0)
	{
		s[j++]=i%10+'0';
		i=i/10;
	}
	s[j]='\0';
//	printf("%s ",s);
	char ch;
	for(int k=0;k<j/2;k++)
	{
		ch=s[k];
		s[k]=s[j-k-1];
		s[j-k-1]=ch;
	}
//	printf("%s \n",s);
	return s;
}
