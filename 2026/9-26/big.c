#include<stdio.h>
int big(int a,int b)//定义比大小函数 
{
	if(a > b)
	{
		printf("%d比%d大\n",a,b);
	}
	else if(a < b)
	{
		printf("%d比%d大\n",b,a);
	}
	else 
	{
		printf("%d等于%d\n",a,b);
	}
}
int main()
{
	while(1)//无限循环 
	{
		int x = 0;
		int y = 0;
		scanf("%d%d",&x,&y);
		if(x==0 && y==0)//结束循环 
		{
			break;
		}
		big(x,y);
	}
	return 0;
}
