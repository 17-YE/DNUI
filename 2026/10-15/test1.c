#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main()
{
	srand((unsigned)time(NULL));
	int a = rand() % 100 + 1;
	int b = 0;
	printf("Please input a number(1~100):\n");
//	printf("%d\n",a);
	while(1)
	{
			scanf("%d",&b);
			if(b == a)
		{
			printf("赢了\n");
			break;
		}
		else if(b < a)
		{
			printf("输入小了\n");
		}
		else
		{
			printf("输入大了\n");
		} 
	}
	return 0;
}
