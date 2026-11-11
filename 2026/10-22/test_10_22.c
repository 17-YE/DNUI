//#include <stdio.h>
//
//int main()
//{
//	int a = 0;
//	scanf("%d", &a);
//
//	if (a == 1)
//	{
//		printf("Monday\n");
//	}
//	else if (a == 2)
//	{
//		printf("Tuesday\n");
//	}
//	else if (a == 3)
//	{
//		printf("Wednesday\n");
//	}
//	else if (a == 4)
//	{
//		printf("Thursday\n");
//	}
//	else if (a == 5)
//	{
//		printf("Friday\n");
//	}
//	else if (a == 6)
//	{
//		printf("Saturday\n");
//	}
//	else if (a == 7)
//	{
//		printf("Sunday\n");
//	}
//
//	return 0;
//}

//#include<stdio.h>
//int main()
//{
//	int a = 0;
//	scanf("%d",&a);
//	switch (a)
//	{
//	case 1:
//		printf("Monday\n");
//		break;
//	case 2:
//		printf("Tuesday\n");	 
//		break;
//	case 3:
//		printf("Wednesday\n");
//		break;
//	case 4:
//		printf("Thursday\n");
//		break;  
//	case 5:
//		printf("Friday\n");
//		break;
//	case 6:
//		printf("Saturday\n");
//		break;
//	case 7:
//		printf("Sunday\n");
//		break;
//	default :
//		printf("Error\n");
//	}
//	return 0;
//}

//#include<stdio.h>
// 
//int main()
//{
//	float x = 0;
//	float y = 0;
//	scanf("%f %f",&x,&y);
//	if((x<=3&&x>=-3)&&(y<=2&&y>=-2))
//	{
//		printf("TURE\n");
//	}
//	else
//	{
//		printf("FALSE\n");
//	}
//	return 0;
// } 

//#include<stdio.h>

//int main()
//{
//	float a = 0;
//	float b = 0;
//	scanf("%f",&a);
//	if(a <= 50)
//	{
//		b = a * 0.15;
//		printf("%.2f",b);
//	}
//	else
//	{
//		b = (a - 50) * 0.1 + 7.5;
//		printf("%.2f",b);
//	}
//	return 0;
// } 

//#include<stdio.h>
//
//int main()
//{
//	int a = 0;
//	scanf("%d",&a);
//	switch(a / 10)
//	{
//		case 10: case 9:
//			printf("A");
//			break;
//		case 8:
//			printf("B");
//			break;
//		case 7:
//			printf("C");
//			break;
//		case 6:
//			printf("D");
//			break;
//		default :
//			printf("E");
//			break;
//	}
//	return 0;
//}

//#include<stdio.h>
//
//int main()
//{
//	int a = 0;
//	int b = 0;
//	scanf("%d %d",&a,&b);
//	if((a >= 1 && a <= 100)&&(b >=1 && b <= 100))
//	{
//		if(a >= 60 && b >= 60)
//		{
//			printf("It is pass");
//		}
//		else
//		{
//			printf("It is not pass");
//		}
//	}
//	else
//	{
//		printf("It is error\n");
//	}
//	return 0;
//}

//#include <stdio.h>
//#include <stdlib.h>
//#include <time.h>
//
//int main()
//{
//	int a = 0;      
//	int b = 0;      
//
//	srand((unsigned)time(NULL));
//	a = rand() % 3 + 1;
//
//	printf("Stone(1), scissors(2), cloth(3)\n");
//	printf("Please input your number (1-3):\n");
//	scanf("%d", &b);
//
//	if (b < 1 || b > 3)
//	{
//		printf("Input error!\n");
//		return 1;
//	}
//
//	switch (a)
//	{
//	case 1:
//		printf("Computer: stone\n");
//		break;
//	case 2:
//		printf("Computer: scissors\n");
//		break;
//	default:
//		printf("Computer: cloth\n");
//		break;
//	}
//
//	switch (b)
//	{
//	case 1:
//		printf("You: stone\n");
//		break;
//	case 2:
//		printf("You: scissors\n");
//		break;
//	default:
//		printf("You: cloth\n");
//		break;
//	}
//
//	if (a == b)
//	{
//		printf("It is a tie!\n");
//	}
//	else if ((a == 1 && b == 2) ||   
//	         (a == 2 && b == 3) ||    
//	         (a == 3 && b == 1))      
//	{
//		printf("Computer wins!\n");
//	}
//	else
//	{
//		printf("You win!\n");
//	}
//
//	return 0;
//}

//#include<stdio.h>
//int main()
//{
//	int i = 0;
//	int a = 20;
//	while(i < 4)
//	{
//		printf("%d\n",a);
//		a += 20;
//		i++;
//	}
//	return 0;
//}

//#include<stdio.h>
//int main()
//{
//	int a = 0;
//	int i = 0;
//	while(i < 100)
//	{
//		i++;
//		a += i;
//	}
//	printf("%d\n",a);
//	return 0;
//}
