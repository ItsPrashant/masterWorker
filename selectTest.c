#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>

char *intToStr(int i,char *);
char worker[]="./worker";
int childAlive[5]={0};

int main()
{
	int noOfWorkers=5,bufferLen=100;
	int p[noOfWorkers][2];
	char buffer[100];
	fd_set readFds;
	FD_ZERO(&readFds);
	struct timeval tv;
	tv.tv_sec=1;
	tv.tv_usec=0;
	printf("I am Master.\n");
	int maxFd=-1;
	char s1[10],s2[10];		
	int check;
	for(int i=1;i<=noOfWorkers;i++)
	{
		printf("i= %d\n",i);
		//printf("value of p[i] before:%d\n",p[i-1][0]);
		const char *chx=(const char*)intToStr(i,s1);
		const char *chn=(const char*)intToStr(i,s2);
		if(pipe(p[i-1])==-1)
		{
			perror("Pipe error\n");
			exit(-1);
		}
		if(maxFd<p[i-1][0])
		maxFd=p[i-1][0];
		//printf("value of p[i] after:%d\n",p[i-1][0]);
		//dup2(p[i-1][0],0);
		//printf("value of p[i] final:%d\n",p[i-1][0]);
		FD_SET(p[i-1][0],&readFds);
		childAlive[i-1]=1;
		printf("pipe[%d][0]=%d \n",i-1,p[i-1][0]);
	
		//printf("Ready to fork\n");
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
//		else
		
		
	}
	//for(int i=0;i<noOfWorkers;i++)
	fd_set cpy;
	cpy=readFds;
	int avWorkers=noOfWorkers;
	while(avWorkers!=0)
	{
	printf("avWorkers=%d\n",avWorkers);

	printf("waiting for select\n");
	//readFds=cpy;
	int r=select(maxFd,&readFds,NULL,NULL,NULL);
	//printf("Returned by select(): %d\n",r);
	if(r==-1)
	{
		perror("select() error\n");
	}
	if(r!=0)
	{	
	//	printf("Something available\n");
		
		for(int i=0;i<noOfWorkers;i++)
		{
			if(childAlive[i]==1)
			{
				if(FD_ISSET(p[i][0],&readFds))
				{
					int nChar=read(p[i][0],buffer,bufferLen);
					printf("Master received: %s from pipe[%d][0]= %d\n",buffer,i,p[i][0]);
					avWorkers--;
					childAlive[i]=0;	
				}
				//else
				//FD_SET(p[i][0],&readFds);
			}
		}
		/*
		for(int i=0;i<noOfWorkers;i++)
		{
			if(childAlive[i]==1)
			FD_SET(p[i][0],&readFds);
		}
		*/
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
