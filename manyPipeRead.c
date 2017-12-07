#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include <sys/types.h>
int main()
{
	int noOfWorkers=3;
	int p[noOfWorkers][2];
	char buffer[100];

	int buffLen=20;
	//int Pipe[2];
	int x;
	for(int i=0;i<noOfWorkers;i++)
	{
		if(pipe(p[i])==-1)
		{
			perror("Pipe error\n");
			exit(-1);
		}
		
		x=fork();
		
		if(x==0)
		{
			char str[20]="Hello papa";
			int len=strlen(str);
			
			str[len]='\0';
			
			//close(Pipe[0]);
			//scanf("%s",str);
			fflush(stdout);
			close(p[i][0]);
			dup2(p[i][1],1);
			
			//read(Pipe[0],str,buffLen);
			printf("hello form child %d",i);
			close(p[i][1]);
			//
		}
	}
	if(x!=0)
	{
		int l;
		for(int i=0;i<3;i++)
		{
			if(l=read(p[i][0],buffer,buffLen))
			{
				buffer[l]='\0';
				printf("Parent reads:%s\n",buffer);
			} 
		}
		wait(NULL);
	}
	return 0;
}
