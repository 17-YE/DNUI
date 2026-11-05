#include<stdio.h>
	int Jia(int a,int b)//定义加法函数 
{	int c = 0;
	c = a + b;
	return c;
}
int main()
{	
	int x = 0;
	int y = 0;
	int z = 0;
	while(1)//无限循环 
	{
		printf("输入0 0结束程序\n请输入两个数字:\n");
		scanf("%d%d",&x,&y);
		if(x==0 && y==0)
		{ 
			break;//结束循环 
		}
		z = Jia(x,y);
		printf("结果是:%d\n",z);
	}
	return 0;
 } 
