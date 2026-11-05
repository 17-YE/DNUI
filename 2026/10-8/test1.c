#include<stdio.h>
int main()
{
//	//1
//	int a = 0;
//	int *par = &a;
//	printf("%d\n",*par);
//	printf("%d\n",&a);
//
//	printf("C Programming\n");
//	printf("\"Bilingual Course\"\n");
//
//	char car = 'a';
//	printf("%d\n",car);
//	//2
//	int num1 = 3;
//	int num2 = 5;
//	//int sum = 0;
//	int sum = num1 + num2;
//	printf("%d\n",sum);
//	float num1 = 3;
//	float num2 = 5;
//	float num3 = num1 + num2;
//	printf("%12f",num3);
//	//3
//	char car1 = 'C';
//	char car2;
//	//scanf("%c",&car1);
//	car2 = car1 + 32;
//	printf("%c",car2);
	
	//4
	const float PI = 3.14;
	float R = 0;
	float a = 0;
	while(1)
	{
		scanf("%f",&R);
		if(R==0)
		{
			break;
		}
		a = PI * R * R;
		printf("%f\n",a);
	}
	return 0;
}
