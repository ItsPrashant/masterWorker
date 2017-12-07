#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
int main()
{
	int buffLen=20;
	char buffer[20];
	int Pipe[2];
	if(pipe(Pipe)==-1)
	{
		perror("Pipe error\n");
		exit(-1);
	}
	
	int x=fork();
	
	if(x==0)
	{
		char str[20]="Hello papa";
		int len=strlen(str);
		
		str[len]='\0';
		
		//close(Pipe[0]);
		//scanf("%s",str);
		fflush(stdout);
	
		dup2(Pipe[1],1);
		
		//read(Pipe[0],str,buffLen);
		printf("%s",str);
		close(Pipe[1]);
		//
	}
	else
	{

		int l;
		if(l=read(Pipe[0],buffer,buffLen))
		{
			buffer[l]='\0';
			printf("Parent reads:%s\n",buffer);
		} 
		wait(NULL);
	}
	return 0;
}
