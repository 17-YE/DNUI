#include <stdio.h>
#include <conio.h> //键盘输入使用
int main(void) 
{
        char key='\0';
        key=getch();
        if(key!='q')
        {
                key=getch();//第二次检测键盘输入 
        if (key==72) 
            {        
               printf("\nup\n");                  
                }         
            else if (key==80)
                {
                       printf("\ndown\n");
                }        
            else if (key==75)
                {
                       printf("\nleft\n");
                }                            
            else if (key==77)
                {
                       printf("\nright\n");
                }        
         }
        else
        {
                printf("\nquit\n");                
         } 
        
}
