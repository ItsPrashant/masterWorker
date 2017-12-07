#include<stdio.h>
#include<unistd.h>
char *intToStr(int i,char *);

int main()
{
	int noOfWorkers=3;
	float x=10,n=3;
	char worker[]="./worker";
	//int i=123;
	//char str[15];
	//char *ch;
	//ch=intToStr(i,str);
	//char *args[]={"./worker","-x","1","-n","1"};
	
	//printf("%s ",ch);
	
	//char temp[19];
	char s1[10],s2[10];		
	int check;
	for(int i=1;i<=noOfWorkers;i++)
	{

		
		const char *chx=(const char*)intToStr(i,s1);
		const char *chn=(const char*)intToStr(i,s2);
//		printf("x=%s n=%s\n",chx,chn);
		
		check=fork();
		if(check==-1)
		perror("fork() failed\n");
		else
		if(check==0)
		{
			printf("Child no.%d\n",i);
			if(execlp	(worker,"worker","-x",chx,"-n",chn,NULL)==-1)
			{
				printf("execlp() failed\n");
			}
		}
	
	}
	wait(NULL);
	printf("Return parent\n");
			
	

	
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
