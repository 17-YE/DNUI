//Q1 
//#include <stdio.h>
//#include <windows.h>
//int main() 
//{
//    int a = 0;
//    printf("请输入倒计时秒数:");
//    scanf("%d", &a);
//    while (a <= 0) 
//    {
//        printf("输入错误，请重新输入！\n");
//        printf("请输入倒计时秒数:");
//        scanf("%d", &a);
//    }
//    while (1) 
//    {
//        printf("\r剩余时间:%5d 秒  ", a); 
//        fflush(stdout);
//        if (a == 0) 
//        {
//            break; 
//        }
//        Sleep(1000);
//        a--;
//    }
//    printf("\n倒计时结束！\n");
//    return 0;
//}

//Q2
//#include <stdio.h>
//#include <conio.h>
//int main() 
//{
//    char key='\0';
//	printf("输入q退出\n");
//    while (1)                      
//    {
//        key = getch();
//        if (key != 'q')
//        {
//            key = getch();
//            if (key == 72) 
//            {        
//                printf("\nup\n");                  
//            }         
//            else if (key == 80)
//            {
//                printf("\ndown\n");
//            }        
//            else if (key == 75)
//            {
//                printf("\nleft\n");
//            }                            
//            else if (key == 77)
//            {
//                printf("\nright\n");
//            }        
//        }
//        else
//        {
//            printf("\nquit\n");
//            break;          
//        }
//    }
//    return 0;                      
//}

//Q3
//#include<stdio.h>
//int main()
//{
//	int a = 0;
//	int i = 1;
//	for(i;i <= 999;i++)
//	{
//		if(i % 10 != 0)
//		{
//			a += i;
//		}
//	}
//	printf("%d",a);
//	return 0;
//}

//Q4
//#include<stdio.h>
//int main()
//{
//	int a = 0;
//	int b = 0;
//	int c = 0;
//	scanf("%d",&a);
//	while(1)
//	{
//		b = a % 10;
//		a /= 10;
//		c += b;
//		if(a == 0)
//		{
//			break;
//		}
//	}
//	printf("%d\n",c);
//	return 0;
//}

//Q5
//#include<stdio.h> 
//int main()
//{
//	int a = 0;
//	int b = 0;
//	for(a = 0;a <= 300;a += 20)
//	{
//		b = (5.0 / 9.0) * (a - 32);
//		printf("%-5d%5d\n",a,b);
//	}
//	return 0 ;
//}
