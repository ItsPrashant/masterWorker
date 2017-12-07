#include<stdio.h>
#include<string.h>
#include<math.h>
#include<stdlib.h>

float n,x;
float strToFloat(char *s);

int main(int argc,char **argv)
{
	
	if(argc!=5)
	{
		for(int i=0;i<argc;i++)
		printf("%s\n",argv[i]);
		perror("Usage:./worker -x [value of x] -n [value of n]");
		exit(1);
	}
	//int p=(int)strToFloat(argv[5]);
	
	
	if(!strcmp(argv[1],"-x"))
	{
		x=strToFloat(argv[2]);
		if(!strcmp(argv[3],"-n"))
		{
			n=strToFloat(argv[4]);
		}
	}
	else
	if(!strcmp(argv[1],"-n"))
	{
		n=strToFloat(argv[2]);
		if(!strcmp(argv[3],"-x"))
		{
			x=strToFloat(argv[4]);
		}
	}
	//printf("x=%f n=%f\n",x,n);
	float factn=1;
	for(int i=1;i<=n;i++)
	factn*=i;
	printf("x^n/n!: %f\n",pow(x,n)/factn);
	return 0;
}

float strToFloat(char *s)
{
	int len=strlen(s);
	float t=0;
	for(int i=0;i<len;i++)
	{
		//printf("%c ",s[i]);
		if(s[i]=='.')
		t=t/pow(10,len-1-i);
		else
		t+=s[i]-'0';
		t=t*10;
	}
	//printf("\n");
	t=t/10;
	return t;
}
