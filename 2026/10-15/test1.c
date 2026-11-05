// #include<stdio.h>
// 
// int main()
// {
// 	//fflush(stdin);
// 	int age = 0;
// 	float height = 0;
// 	char car;
// 	float weight = 0;
// 	float bmi = 0;
// 	
// 	printf("姓名：叶贵骞\n");
// 	scanf("%d",&age);
// 	printf("年龄：%d\n",age);
// 	scanf("%f",&height);
// 	printf("身高：%.1f\n",height);
// 	scanf(" %c",&car);
// 	printf("性别:%c\n",car);
// 	scanf("%f",&weight);
// 	printf("体重：%.1f\n",weight);
// 	bmi = weight / (height * height);
// 	printf("BMI:%.1f",bmi);

//#include<stdio.h>
//
//int main()
//{
//	int a = 0;
//	scanf("%d",&a);
//	if(a % 2 == 0)
//	{
//		printf("是偶数"); 
//	}
//	else 
//	{
//		printf("是奇数");
//	}
//	return 0;
//}

//#include <stdio.h>
//
//int main()
//{
//	int a = 0;
//	scanf("%d",&a);
//	switch(a % 2)
//	{
//		case 0:
//			printf("偶数\n");
//			break;
//		default:
//			printf("奇数\n");
//			break;
//	}
//	return 0;
//}

//#include<stdio.h>
//
//int main()
//{
//	int a = 0;
//	scanf("%d",&a);
//	if(1 <= a)
//	{
//		if(10 >= a)
//		{
//			printf("OK\n");
//		}
//		else
//		{
//			printf("NG\n");
//		}
//	}
//	else
//	{
//		printf("NG\n");
//	}
//}

//#include<stdio.h>
//
//int main()
//{
//	int a = 0;
//	int b = 0;
//	int c = 0;
//	scanf("%d %d %d",&a,&b,&c);
//	if(a > b)
//	{
//		if(b > c)
//		{
//			printf("%d>%d>%d",a,b,c);
//		}
//		else
//		{
//			printf("%d>%d>%d",a,c,b);
//		}
//	}
//	else if(a > c)
//	{
//		if(c > b)
//		{
//			printf("%d>%d>%d",a,c,b);
//		}
//	}
//	else if(c > a)
//	{
//		if(a > b)
//		{
//			printf("%d>%d>%d",c,a,b);
//		}
//		else
//		{
//			printf("%d>%d>%d",c,b,a);
//		}
//	}
//	return 0;
//}

#include<stdio.h>

int main()
{
	int a = 0;
	scanf("%d",&a);
	if(1 <= a && 10 >= a)
	{
		printf("OK\n");
	}
	else
	{
		printf("NG\n");
	}
	return 0;
}
