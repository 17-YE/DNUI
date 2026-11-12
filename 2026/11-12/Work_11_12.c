//Q1
//#include <stdio.h>
//#include <conio.h>      // 用于 _kbhit() 和 _getch()
//#include <windows.h>    // 用于 SetConsoleCursorPosition 和开启颜色支持
//
//// 玩家的坐标设置
//int pl_x = 15, pl_y = 7;
//int current_color = 7; // 初始颜色为 7 (WHITE)
//
//// 开启 Windows 终端 ANSI 颜色支持 (关键：否则颜色代码会变成乱码)
//void enableAnsi() {
//    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
//    DWORD mode = 0;
//    GetConsoleMode(hOut, &mode);
//    SetConsoleMode(hOut, mode | 0x0004); // ENABLE_VIRTUAL_TERMINAL_PROCESSING
//}
//
//// 移动光标 (使用 Windows API 精准定位)
//void moveCursor(int x, int y) {
//    COORD pos = { (SHORT)x, (SHORT)y };
//    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
//}
//
//// 设置颜色 (题目要求)
//void setColor(int col) {
//    if (col < 0 || col > 7) col = 7; 
//    printf("\x1b[3%dm", col);
//}
//
//// 重置颜色为默认
//void resetColor() {
//    printf("\x1b[0m");
//}
//
//// 绘制角色 P (只绘制新位置，不擦除旧位置，保留拖影)
//void drawPlayer() {
//    moveCursor(pl_x, pl_y);
//    setColor(current_color);
//    printf("P");
//    resetColor(); 
//}
//
//int main(void) {
//    enableAnsi(); // 开启颜色支持
//    
//    int key = 0;
//
//    // 显示操作说明
//    printf("Use arrow keys to move, press 0-7 to change color, press 'q' to quit.\n");
//    
//    // 初始绘制
//    drawPlayer();
//
//    while (key != 'q' && key != 'Q') {
//        if (_kbhit()) { // 检测是否有按键输入
//            key = _getch(); // 获取按键
//
//            // --- 处理方向键 ---
//            // 修复了原图中没处理双字节的 Bug
//            if (key == 0 || key == 224) {
//                key = _getch(); // 读取方向键的真实键值
//                switch (key) {
//                    case 72: pl_y--; break; // 上
//                    case 80: pl_y++; break; // 下
//                    case 75: pl_x--; break; // 左
//                    case 77: pl_x++; break; // 右
//                }
//            }
//            // --- 处理数字键 0-7 变色 ---
//            else if (key >= '0' && key <= '7') {
//                current_color = key - '0'; // 更新颜色变量
//            }
//
//            // 移动光标并绘制玩家 (每次都会在当前位置留下一个 P)
//            drawPlayer();
//        }
//        
//        Sleep(10); // 防止 CPU 占用率 100%
//    }
//
//    // 退出程序前恢复默认状态
//    resetColor();
//    moveCursor(0, 15); 
//    printf("\nProgram exited.\n");
//
//    return 0;
//}

//Q2  26003150217  叶贵骞
//#include <stdio.h>
//#include <conio.h>      // 用于 _kbhit() 和 _getch()
//#include <windows.h>    // 用于 SetConsoleCursorPosition 和开启颜色支持
//
//// 玩家的坐标设置
//int pl_x = 15, pl_y = 7;
//int current_color = 7; // 初始颜色为 7 (WHITE)
//
//// 开启 Windows 终端 ANSI 颜色支持
//void enableAnsi() {
//    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
//    DWORD mode = 0;
//    GetConsoleMode(hOut, &mode);
//    SetConsoleMode(hOut, mode | 0x0004); // ENABLE_VIRTUAL_TERMINAL_PROCESSING
//}
//
//// 移动光标 (使用 Windows API 精准定位)
//void moveCursor(int x, int y) {
//    COORD pos = { (SHORT)x, (SHORT)y };
//    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
//}
//
//// 设置颜色
//void setColor(int col) {
//    if (col < 0 || col > 7) col = 7; 
//    printf("\x1b[3%dm", col);
//}
//
//// 重置颜色
//void resetColor() {
//    printf("\x1b[0m");
//}
//
//// 核心更新函数：包含擦除旧位置和绘制新位置的逻辑
//void updatePlayer(int old_x, int old_y) {
//    // 1. 如果坐标发生了变化，先回到旧位置打印空格进行擦除
//    if (old_x != pl_x || old_y != pl_y) {
//        moveCursor(old_x, old_y);
//        printf(" "); // 打印空格擦除旧痕迹
//    }
//    
//    // 2. 移动到新位置并绘制
//    moveCursor(pl_x, pl_y);
//    setColor(current_color);
//    printf("P");
//    resetColor(); 
//}
//
//int main(void) {
//    enableAnsi(); 
//    
//    int key = 0;
//
//    printf("Use arrow keys to move, press 0-7 to change color, press 'q' to quit.\n");
//    
//    // 初始绘制
//    moveCursor(pl_x, pl_y);
//    setColor(current_color);
//    printf("P");
//    resetColor(); 
//
//    while (key != 'q' && key != 'Q') {
//        if (_kbhit()) { 
//            key = _getch(); 
//
//            // 记录移动前的旧坐标
//            int old_x = pl_x;
//            int old_y = pl_y;
//
//            // --- 处理方向键 ---
//            if (key == 0 || key == 224) {
//                key = _getch(); 
//                switch (key) {
//                    case 72: pl_y--; break; // 上
//                    case 80: pl_y++; break; // 下
//                    case 75: pl_x--; break; // 左
//                    case 77: pl_x++; break; // 右
//                }
//            }
//            // --- 处理数字键 0-7 变色 ---
//            else if (key >= '0' && key <= '7') {
//                current_color = key - '0'; 
//            }
//
//            // 调用更新函数，传入旧坐标用于擦除
//            updatePlayer(old_x, old_y);
//        }
//        
//        Sleep(10); 
//    }
//
//    resetColor();
//    moveCursor(0, 15); 
//    printf("\nProgram exited.\n");
//
//    return 0;
//} 

//Q4
//#include <stdio.h>
//int findMax(int num1,int num2)
//{
//	int max;
//    if (num1 > num2) 
//	{
//        max = num1;
//    } 
//	else 
//	{
//	    max = num2;
//    }
//    return max;
//}
//int main() {
//    int num1 = 0, num2 = 0;
//    int max = 0;
//    scanf("%d %d",&num1,&num2);
//    max = findMax(num1,num2);
//    printf("The maximum number between %d and %d is %d.\n", num1, num2, max);
//    return 0;
//}

//Q4
//#include <stdio.h>
//#include <conio.h>      // 用于 _kbhit() 和 _getch()
//#include <windows.h>    // 用于 SetConsoleCursorPosition 和开启颜色支持
//
//// 颜色常量定义
//#define BLACK 0
//#define RED 1
//#define GREEN 2
//#define YELLOW 3
//#define BLUE 4
//#define MAGENTA 5
//#define CYAN 6
//#define WHITE 7
//
//// 全局状态变量
//int pl_x = 15, pl_y = 7;     // 玩家初始坐标
//int current_color = WHITE;   // 玩家初始颜色
//int gold = 0;                // 全局变量：记录当前金币数量
//
//// 开启 Windows 终端 ANSI 颜色支持
//void enableAnsi() {
//    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
//    DWORD mode = 0;
//    GetConsoleMode(hOut, &mode);
//    SetConsoleMode(hOut, mode | 0x0004); 
//}
//
//// 移动光标
//void moveCursor(int x, int y) {
//    COORD pos = { (SHORT)x, (SHORT)y };
//    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
//}
//
//// 设置颜色
//void setColor(int col) {
//    if (col < 0 || col > 7) col = WHITE; 
//    printf("\x1b[3%dm", col);
//}
//
//// 重置颜色
//void resetColor() {
//    printf("\x1b[0m");
//}
//
//// 绘制玩家（包含擦除旧位置的逻辑）
//void drawPlayer(int old_x, int old_y) {
//    if (old_x != pl_x || old_y != pl_y) {
//        moveCursor(old_x, old_y);
//        printf(" "); // 擦除旧位置
//    }
//    moveCursor(pl_x, pl_y);
//    setColor(current_color);
//    printf("P");
//    resetColor(); 
//}
//
//// ================= Question 3 核心函数 =================
//int putGold() {
//    // 1. 光标向左移动一格
//    moveCursor(pl_x - 1, pl_y);
//    
//    // 2. 设置颜色为黄色并打印 'G'
//    setColor(YELLOW);
//    printf("G");
//    resetColor();
//    
//    // 3. 全局金币数量加 1
//    gold++;
//    
//    // 4. 在左下方更新显示（修改为 y=20，如果你的窗口更小，可以改成 15 或 18）
//    moveCursor(0, 20); 
//    setColor(WHITE);
//    printf("Gold count: %d.   ", gold); // 后面留空格清除残影
//    resetColor();
//    
//    // 5. 返回当前金币数量
//    return gold;
//}
//// =======================================================
//
//int main(void) {
//    enableAnsi(); 
//    int key = 0;
//
//    // 隐藏光标
//    printf("\x1b[?25l"); 
//
//    printf("Use arrow keys to move, Press 'q' to quit.\n");
//    
//    // 初始化
//    drawPlayer(pl_x, pl_y);
//    // 初始化金币显示位置 y=20
//    moveCursor(0, 20); printf("Gold count: 0.   "); 
//
//    while (key != 'q' && key != 'Q') {
//        if (_kbhit()) { 
//            key = _getch(); 
//            int old_x = pl_x;
//            int old_y = pl_y;
//
//            // --- 处理方向键 ---
//            if (key == 0 || key == 224) {
//                key = _getch(); 
//                switch (key) {
//                    case 72: pl_y--; break; // 上
//                    case 80: pl_y++; break; // 下
//                    case 75: pl_x--; break; // 左
//                    case 77: pl_x++; break; // 右
//                }
//                drawPlayer(old_x, old_y);
//            }
//            // --- 处理数字键变色 ---
//            else if (key >= '0' && key <= '7') {
//                current_color = key - '0'; 
//            }
//            // --- 处理 'g' 键放置金币 ---
//            else if (key == 'g' || key == 'G') {
//                putGold(); 
//                
//                // 重新绘制 P
//                moveCursor(pl_x, pl_y);
//                setColor(current_color);
//                printf("P");
//                resetColor();
//            }
//        }
//        Sleep(10); 
//    }
//
//    // 恢复光标显示，退出提示放在 y=22
//    printf("\x1b[?25h"); 
//    moveCursor(0, 22); 
//    printf("\nProgram exited.\n");
//
//    return 0;
//} 

//Q5
//#include<stdio.h>
//int max(int a,int b,int c)
//{
//	if(a >= b && a >= c)
//	{
//		return a;
//	}
//	else if(b >= a && b >= c)
//	{
//		return b;
//	}
//	else
//	{
//		return c;
//	}
//}
//int main()
//{
//	int a, b, c;
//	scanf("%d %d %d",&a,&b,&c);
//	printf("%d",max(a,b,c));
//	return 0;
//}
