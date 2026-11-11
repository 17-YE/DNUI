#include <stdio.h>
#include <windows.h> // 包含Sleep()函数的头文件
int main() {
    int seconds;    
    // 提示用户输入倒计时秒数
    printf("请输入倒计时秒数: ");
    scanf("%d", &seconds);    
    // 确保输入的是正数
    if (seconds <= 0) {
        printf("请输入一个正整数!\n");
        return 1;
    }    
    // 倒计时循环
    while (seconds >= 0) {
        // 清除当前行并打印剩余时间
        printf("\r剩余时间: %5d 秒", seconds);
        fflush(stdout); // 刷新输出缓冲区，确保立即显示
        
        // 如果倒计时结束，退出循环
        if (seconds == 0) {
            break;
        }        
        // 等待1秒
        Sleep(1000); // Sleep函数的参数单位是毫秒，1000毫秒 = 1秒        
        // 时间递减
        seconds--;
    }    
    // 倒计时结束提示
    printf("\n倒计时结束!\n");    
    return 0;
}
