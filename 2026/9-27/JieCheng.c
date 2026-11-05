#include<stdio.h>
long long jie(int a)
{	
	long long c = 1;
	for(int b = 1;b <= a;b++)
	{
		c = c * b;
	}
	return c;
}
int main()
{
	int x = 0;
	long long y = 0;
	scanf("%d",&x);
	y = jie(x);
	printf("%lld",x,y);
	return 0;
}
