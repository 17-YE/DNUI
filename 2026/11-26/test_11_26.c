//#include <stdio.h>
//int main()
//{
//	int x = 8;
//	int* ptr=&x;
//	printf("%d",x);
//	printf("%p\n",&x);
//	printf("%p\n",ptr);
//	printf("%p\n",&ptr);
//	printf("%d\",*ptr);
//	printf("%d\n",*ptr*2);
//	return 0;
//} 

//#include <stdio.h>
//int main()
//{
//	float a[10]={2,4,6,8,10,12,14,16,18,20};
//	float* ptr1=&a[2];
//	printf("%p\n",ptr1);
//	printf("%p\n",&ptr1);
//	printf("%.f\n",*ptr1);
//	a[2] -= 1;
//	ptr1+=1;
//	printf("%.f\n",*ptr1);
//	int i = 0;
//	for(i=0;i<10;i++)
//	{
//		printf("%d\n",a[i]);
//	}
//	return 0;
//}

//#include <stdio.h>
//int main()
//{
//	int a[10]={2,4,6,8,10,12,14,16,18,20};
//	int* ptr1=a;
//	int i = 0;
//	for(i=0;i<10;i++)
//	{
//		printf("%d\n",a[i]);
//		printf("%d\n",*(ptr1+i));
//	}
//	return 0;
//}

//#include <stdio.h>
//int main()
//{
//	int a[6]={3,4,1,6,2,9};
//	int* ptr1=a;
//	int i = 0;
//	for(i=0;i<6;i++)
//	{
//		printf("%d \n",a[i]);
//		printf("%d \n",*(ptr1+i));
//	}
//		for(i=0;i<6;i++)
//	{
//		if(a[i]==6)
//		{
//			printf("%d\n",i);
//		}
//	}
//	return 0;
//}

//#include <stdio.h>
//int main(void)
//{
//	int a[6]={3,4,1,6,2,9};
//	int* ptr1=a;
//	int i = 0;
//	int max=a[0];
//	for(i=0;i<6;i++)
//	{
//		printf("%d\n",*(ptr1+i));
//		if(*(ptr1+i)>max)
//		{
//			max=*(ptr1+i);
//		}		
//	}
//	printf("%d\n",max);
//	return 0;
//}

#include<stdio.h>
int main()
{
	int a = 0;
	int b = 0;
	int c = 0;
	int *p1=&a;
	int *p2=&b;
	scanf("%d %d",p1,p2);
	c=*p1;
	*p1=*p2;
	*p2=c;
	printf("%d %d",a,b);
	return 0;
}
