/*
 * ============================================================
 *  三级项目：掘金者扩展版
 *  扩展方向：收集类 + 生存类
 *  文件名称：Work_11_26.c
 *
 *  玩法说明：
 *    Y  玩家              G  金币（+1 分）
 *    $  宝石（+3 分）      +  药水（+1 点生命）
 *    Z  僵尸（碰到 -1 点生命）
 *
 *  普通三关：简单 / 中等 / 困难，每关 90 秒
 *  隐藏地狱关：开始菜单依次按 W W S S A D A D B A 解锁，
 *              解锁后按 0 进入
 *
 *  程序整体结构（从上到下）：
 *    1. 宏定义与全局数据：地图尺寸、窗口尺寸、关卡/地狱参数、游戏状态
 *    2. 基础控制台函数：光标、颜色、清屏、按安全宽度输出文字
 *    3. 键盘输入：非阻塞读键、清空输入缓冲
 *    4. 初始化：设置窗口大小、标题、GBK 编码并检查窗口是否够大
 *    5. 地图生成：随机 DFS 迷宫、打通环路、随机空地查找
 *    6. 画面绘制：用 CHAR_INFO 一次画完整张地图，再画 HUD 与提示
 *    7. 菜单/秘籍：开始界面与 W W S S A D A D B A 状态机
 *    8. 游戏逻辑：关卡初始化、拾取、僵尸 AI、玩家移动、胜负判断
 *    9. 主循环：菜单 -> 一局游戏 -> 回菜单，直到选择退出
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h>
#include <windows.h>

/* 方向键在 getch() 中返回的扩展键值（先读到 0/224，再读到这些值） */
#define KEY_UP    72
#define KEY_DOWN  80
#define KEY_LEFT  75
#define KEY_RIGHT 77

/* ---------------- 地图与 HUD 布局 ----------------
 * 地图占 0 ~ MAP_H-1 行，紧接着是 4 行 HUD：
 *   HUD_Y   ：得分 / 关卡 / 时间 / 生命
 *   HUD2_Y  ：目标分数 / 难度 / 按键提示
 *   MSG_Y   ：吃到道具、被撞等短消息
 *   HINT_Y  ：金币提示（默认隐藏，连按 3 个 0 开关）
 * 窗口高度 = 地图 43 行 + HUD 4 行 + 1 行余量 = 48 行。
 */
#define MAP_W 86
#define MAP_H 43

#define HUD_Y   MAP_H
#define HUD2_Y  (MAP_H + 1)
#define MSG_Y   (MAP_H + 2)
#define HINT_Y  (MAP_H + 3)

/* 控制台缓冲区与窗口尺寸；宽度 90 留给 86 列地图左右各 2 列余量 */
#define CONSOLE_W 90
#define CONSOLE_H 48

/* 结算画面相对于旧 36 行窗口的垂直偏移，让文字在新窗口中居中。
 * (48 - 36) / 2 = 6，所以所有结算文字统一加 UI_TOP。 */
#define UI_TOP ((CONSOLE_H - 36) / 2)

/* ---------------- 普通三关参数 ---------------- */
#define MAX_HP       5      /* 普通关最大生命 */
#define MAX_ENEMIES  32     /* 僵尸数组上限，必须 >= 地狱 32 只 */
#define TOTAL_LEVELS 3      /* 普通关总数 */

/* 普通关道具数量：按 86×43 面积约等于旧版 2 倍 */
#define GOLD_COUNT   26
#define GEM_COUNT    8
#define POTION_COUNT 4

/* ---------------- 隐藏地狱关参数 ----------------
 * 地狱关明显更难：僵尸更多、移动更快、生命更少、目标更高。
 * HELL_INTERVAL = 5 表示每 5 帧（5×50ms = 250ms）移动一次。
 * HELL_RANDOM_PCT = 5 表示每次只有 5% 概率随机乱走，其余时间紧追玩家。
 */
#define HELL_ZOMBIES    32
#define HELL_GOLD       12
#define HELL_GEM        4
#define HELL_POTION     2
#define HELL_HP         3
#define HELL_TIME       75
#define HELL_GOAL       40
#define HELL_INTERVAL   5
#define HELL_RANDOM_PCT 5

/* 颜色编号，与 ygq_color() 的下标一一对应：
 * 0 黑、1 红、2 绿、3 黄、4 蓝、5 品红、6 青、7 白 */
enum { BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE };

/* 游戏状态机。主循环 ygq_play_game() 根据状态决定读哪个键、
 * 是否继续倒计时、是否重绘画面。 */
enum {
    ST_PLAYING,      /* 正在游戏：可移动、计时、僵尸移动 */
    ST_PAUSED,       /* 暂停：只响应 P（继续）和 Q（回菜单） */
    ST_LEVEL_CLEAR,  /* 普通关通过：等待空格进入下一关/通关画面 */
    ST_GAME_OVER,    /* 失败：等待 R 重开或 Q 回菜单 */
    ST_VICTORY,      /* 普通三关全部通过：等待 R/Q */
    ST_HELL_CLEAR    /* 地狱关通过：等待 R/Q */
};

/* 僵尸结构体
 *   x, y ：当前所在格子（地图坐标，0 起）
 *   tick ：移动计时器，每帧 +1；达到 enemy_interval 时移动一格并清零。
 *          不同僵尸初始 tick 随机，避免所有僵尸同一帧一起动。 */
typedef struct {
    int x;
    int y;
    int tick;
} Enemy;

/* 每关配置
 *   name       ：显示在 HUD 上的难度名（简单/中等/困难）
 *   zombies    ：本关僵尸数量
 *   interval   ：移动间隔（帧，1 帧 = 50ms）
 *   random_pct ：每次移动时随机乱走的概率（百分比）
 *   goal       ：过关所需的本关分数 */
typedef struct {
    const char *name;
    int zombies;
    int interval;
    int random_pct;
    int goal;
} LevelCfg;

/* 三关配置表，下标 0~2 对应第 1~3 关。
 * 字段顺序：难度名、僵尸数、移动间隔（帧）、随机比例（%）、目标分。
 * 说明：僵尸数按地图面积翻倍后为 8/12/16；
 *       移动间隔越小僵尸越快；随机比例越低越会死追玩家。 */
static const LevelCfg level_cfg[TOTAL_LEVELS] = {
    { "简单", 8, 11, 40, 25 },
    { "中等", 12, 9, 20, 40 },
    { "困难", 16, 7,  8, 60 }
};

/* ---------------- 全局变量 ---------------- */
/* 地图内容，每个格子存一个字符：
 *   '#' 墙   ' ' 空地   'G' 金币   '$' 宝石   '+' 药水
 * 玩家和僵尸不写进 map，由绘制函数临时叠加，避免移动时改坏地图。 */
static char map[MAP_H][MAP_W + 1];

/* 玩家坐标、总分、本关得分、生命 */
static int pl_x = 1, pl_y = 1;
static int score = 0;
static int level_score = 0;
static int hp = MAX_HP;
static int hp_max = MAX_HP;

/* 当前关卡号（1~3）、剩余秒数、本关目标分 */
static int level = 1;
static int time_left = 90;
static int goal = 25;

/* 游戏状态、受伤无敌帧数、总帧计数器（约 20 帧 = 1 秒） */
static int state = ST_PLAYING;
static int invincible = 0;
static int frame = 0;

/* HUD 消息及其剩余显示帧数；msg_timer 到 0 时清空 message */
static int msg_timer = 0;
static char message[160] = "";

/* 僵尸数组与当前数量；移动间隔和随机比例在每关初始化时从 level_cfg 读取 */
static Enemy enemies[MAX_ENEMIES];
static int enemy_count = 0;
static int enemy_interval = 9;
static int enemy_random_pct = 20;

/* 过关时把剩余秒数换算成分数，先存这里再显示到过关画面 */
static int level_bonus = 0;

/* 标准输出句柄，所有控制台 API 都通过它操作 */
static HANDLE hOut;

/* 地狱关标记：1 表示当前在地狱模式；hell_unlocked 只在本次运行内有效 */
static int hell_mode = 0;
static int hell_unlocked = 0;

/* 当前难度名，指向 level_cfg[].name 或字符串 "地狱" */
static const char *difficulty_name = "简单";

/* ---------------- 金币提示状态 ----------------
 * hint_on    ：提示开关，默认关闭；连按 3 个 0 切换
 * zero_count ：连续按 0 的计数
 * hint_dir   ：第一段方向（0 上 / 1 下 / 2 左 / 3 右），-1 表示没有金币
 * hint_steps ：第一段要走的步数
 * hint_dirty ：1 表示需要重新计算；玩家移动或吃到金币时置 1
 */
static int hint_on = 0;
static int zero_count = 0;
static int hint_dir = -1;
static int hint_steps = 0;
static int hint_dirty = 1;

/* ============================================================
 *  基础控制台函数
 *  这一组负责光标、颜色、清屏和文字输出，是上层绘制函数的基础。
 *  除 ygq_clear_rect 外都不涉及游戏逻辑。
 * ============================================================ */

/* ygq_cursor：把控制台光标移动到 (x, y)。
 * 参数：x 为列（0 起），y 为行（0 起），左上角是 (0, 0)。
 * 说明：后续的 printf / putchar 会从光标位置开始写。
 *       地图本身用 WriteConsoleOutputW 按坐标直接写，不依赖光标。 */
static void ygq_cursor(int x, int y)
{
    COORD pos;
    pos.X = (SHORT)x;
    pos.Y = (SHORT)y;
    SetConsoleCursorPosition(hOut, pos);
}

/* ygq_color：设置后续文字的颜色。
 * 参数：col 取 0~7，对应 BLACK/RED/GREEN/YELLOW/BLUE/MAGENTA/CYAN/WHITE。
 * 说明：颜色表里除白色外都加了 FOREGROUND_INTENSITY（高亮），
 *       在 cmd 默认配色下更醒目；超出 0~7 时忽略，不改变当前颜色。 */
static void ygq_color(int col)
{
    static const WORD attr[8] = {
        0,
        FOREGROUND_RED | FOREGROUND_INTENSITY,
        FOREGROUND_GREEN | FOREGROUND_INTENSITY,
        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
        FOREGROUND_BLUE | FOREGROUND_INTENSITY,
        FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
        FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
    };
    if (col >= 0 && col < 8)
        SetConsoleTextAttribute(hOut, attr[col]);
}

/* ygq_clear_screen：清空整个控制台缓冲区并回到左上角。
 * 实现：先用 FillConsoleOutputCharacter 填空格，再用
 *       FillConsoleOutputAttribute 恢复当前属性，最后光标归位。
 * 说明：比逐行 printf 空格更快，也不会因为行末换行而滚动屏幕。 */
static void ygq_clear_screen(void)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    COORD home;
    DWORD cells, written;

    home.X = 0;
    home.Y = 0;
    if (!GetConsoleScreenBufferInfo(hOut, &csbi))
        return;
    cells = (DWORD)csbi.dwSize.X * (DWORD)csbi.dwSize.Y;
    FillConsoleOutputCharacter(hOut, ' ', cells, home, &written);
    FillConsoleOutputAttribute(hOut, csbi.wAttributes, cells, home, &written);
    SetConsoleCursorPosition(hOut, home);
}

/* ygq_show_cursor：显示或隐藏光标。
 * 参数：visible 非 0 显示，0 隐藏。
 * 说明：游戏过程中隐藏光标，退出前在 main() 中重新显示，
 *       避免光标在地图上闪烁影响画面。 */
static void ygq_show_cursor(int visible)
{
    CONSOLE_CURSOR_INFO info;
    if (GetConsoleCursorInfo(hOut, &info)) {
        info.bVisible = visible ? TRUE : FALSE;
        SetConsoleCursorInfo(hOut, &info);
    }
}

/* ygq_safe_width：返回当前一行里可以安全写入的列数。
 * 返回值：min(CONSOLE_W - 2, 实际缓冲区宽度 - 1)。
 *
 * 为什么需要它：
 *   程序启动时虽然执行了 mode con cols=90，但 Windows Terminal
 *   可能把缓冲区压成 86/87 列。如果仍按 88 列补空格，写到最后一列
 *   后会自动换行，导致 HUD 和菜单整体下移、越写越乱。
 *   这里每次绘制都重新读取实际宽度，并始终留出最后一列，
 *   无论窗口是 90 列还是 86 列都不会触发换行。 */
static int ygq_safe_width(void)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int w = CONSOLE_W - 2;

    if (GetConsoleScreenBufferInfo(hOut, &csbi)) {
        if (csbi.dwSize.X - 1 < w)
            w = csbi.dwSize.X - 1;
    }
    if (w < 0)
        w = 0;
    return w;
}

/* ygq_clear_line：用空格覆盖第 y 行，用于清除 HUD/提示行。
 * 参数：y 为行号。
 * 说明：用 FillConsoleOutputCharacter 直接写缓冲区，只写
 *       ygq_safe_width() 列，不经过 putchar，因此不会在行末换行。
 *       金币提示关闭时就是靠它把第 4 行整行擦干净的。 */
static void ygq_clear_line(int y)
{
    COORD pos;
    DWORD written;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int w = ygq_safe_width();

    ygq_color(WHITE);
    if (!GetConsoleScreenBufferInfo(hOut, &csbi))
        return;
    pos.X = 0;
    pos.Y = (SHORT)y;
    FillConsoleOutputCharacter(hOut, ' ', (DWORD)w, pos, &written);
    FillConsoleOutputAttribute(hOut, csbi.wAttributes, (DWORD)w, pos, &written);
}

/* ygq_clear_rect：用空格覆盖一个矩形区域。
 * 参数：左上角 (x, y)，宽 w 列，高 h 行。
 * 用途：暂停时先擦掉地图中间一小块，再写“暂 停”提示，
 *       让提示文字不会被地图的 '#' 干扰。 */
static void ygq_clear_rect(int x, int y, int w, int h)
{
    int i, j;
    ygq_color(WHITE);
    for (j = 0; j < h; j++) {
        ygq_cursor(x, y + j);
        for (i = 0; i < w; i++)
            putchar(' ');
    }
}

/* ygq_text：在指定位置 (x, y) 用颜色 col 输出字符串 s。
 * 参数：x 列、y 行、col 颜色 0~7、s 以 '\0' 结尾的 GBK 字符串。
 * 说明：只用于菜单、结算等静态文字；游戏中的地图和 HUD
 *       分别用 ygq_draw_map / ygq_draw_hud 绘制。 */
static void ygq_text(int x, int y, int col, const char *s)
{
    ygq_color(col);
    ygq_cursor(x, y);
    printf("%s", s);
}

/* ygq_text_center：在第 y 行水平居中输出字符串。
 * 参数：y 行号、col 颜色、s 字符串。
 *
 * 居中算法：x = (CONSOLE_W - strlen(s)) / 2。
 *   GBK 编码中 1 个汉字占 2 字节，控制台显示宽度也正好是 2 列，
 *   所以 strlen() 的字节数恰好等于屏幕列数，可以直接用来算居中。
 *   若字符串比窗口还宽，x 会被夹到 0，至少保证从最左边开始显示。 */
static void ygq_text_center(int y, int col, const char *s)
{
    int w = (int)strlen(s);
    int x = (CONSOLE_W - w) / 2;

    if (x < 0)
        x = 0;
    ygq_text(x, y, col, s);
}

/* ygq_separator：在第 y 行画一条与窗口宽度匹配的绿色 '=' 分隔线。
 * 参数：y 行号。
 * 说明：长度 = ygq_safe_width() - 2，从第 2 列开始画，
 *       90 列窗口下是 86 个 '='，窗口被压窄时会自动缩短，
 *       不会写到最后一列而换行。 */
static void ygq_separator(int y)
{
    char line[CONSOLE_W + 1];
    int i, n = ygq_safe_width() - 2;

    if (n < 0)
        n = 0;
    for (i = 0; i < n; i++)
        line[i] = '=';
    line[i] = '\0';
    ygq_text(2, y, GREEN, line);
}

/* ============================================================
 *  键盘输入
 *  使用 conio.h 的 kbhit/getch 做非阻塞读键；
 *  方向键需要处理 0/224 前缀，具体见 ygq_read_key。
 * ============================================================ */

/* ygq_read_key：非阻塞读取一个按键。
 * 返回值：没有按键时返回 0；普通键返回 ASCII 码；
 *         方向键返回 KEY_UP/KEY_DOWN/KEY_LEFT/KEY_RIGHT（72/80/75/77）。
 * 说明：方向键在 getch() 中会先返回 0 或 224 作为前缀，
 *       此时必须再取一次才是真正的键值。 */
static int ygq_read_key(void)
{
    int key = 0;

    if (kbhit()) {
        key = getch();
        /* 方向键会先返回 0 或 224，需要再取一次真实键值 */
        if (key == 0 || key == 224)
            key = getch();
    }
    return key;
}

/* ygq_flush_input：清空键盘缓冲区。
 * 用途：从菜单进入游戏、从结算画面重开时，把之前连按但还没处理
 *       的按键丢掉，避免“上一局的方向键”影响新一局。
 * 注意：菜单刚显示时也会调用一次，所以秘籍不要按得太早，
 *       否则前几个键可能被这里清掉。 */
static void ygq_flush_input(void)
{
    while (kbhit())
        getch();
}

/* ============================================================
 *  窗口与控制台初始化
 *  启动时设置窗口大小、标题、代码页，并检查窗口是否够大。
 * ============================================================ */

/* ygq_check_window_size：检查控制台可视窗口是否足够大。
 * 要求：宽 >= MAP_W（86），高 >= MAP_H + 4（47，地图 43 + HUD 4）。
 *
 * 行为：
 *   - 够大：立即返回；
 *   - 不够：清屏并显示“窗口太小”，告诉用户当前尺寸和最小尺寸，
 *           每 200ms 重新检查一次，用户调整好窗口后自动继续；
 *   - 用户按 Esc：强制继续（画面可能显示不全）。
 * 说明：在 Windows Terminal 下 srWindow 有时会误报尺寸，
 *       导致本函数无法发现底部 HUD 被裁，属于已知限制。 */
static void ygq_check_window_size(void)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    char buf[100];
    int w, h, last_w = -1, last_h = -1, key;

    for (;;) {
        if (!GetConsoleScreenBufferInfo(hOut, &csbi))
            return;
        w = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        h = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        if (w >= MAP_W && h >= MAP_H + 4)
            return;

        if (w != last_w || h != last_h) {
            ygq_clear_screen();
            ygq_text_center(4, YELLOW, "窗口太小，地图显示不全");
            ygq_text_center(6, WHITE, "请最大化窗口，或按住 Ctrl 滚动滚轮缩小字体");
            sprintf(buf, "当前窗口 %d x %d，至少需要 %d x %d",
                    w, h, MAP_W, MAP_H + 4);
            ygq_text_center(8, CYAN, buf);
            ygq_text_center(10, WHITE, "调整好窗口后会自动继续（按 Esc 强制继续）");
            last_w = w;
            last_h = h;
        }

        key = ygq_read_key();
        if (key == 27)
            return;
        Sleep(200);
    }
}

/* ygq_init_console：初始化控制台环境，程序启动时调用一次。
 *
 * 依次完成：
 *   1. 关闭 stdout 缓冲，保证 printf 立即输出到当前光标位置；
 *   2. 用 mode con 把窗口设为 90×48；
 *   3. 取得标准输出句柄 hOut；
 *   4. 设置窗口标题；
 *   5. 把控制台输入/输出代码页设为 936（GBK），中文才不会乱码；
 *   6. 隐藏光标、清屏、检查窗口尺寸。
 *
 * 为什么第 1 步很重要：
 *   地图用 WriteConsoleOutputW 直接写屏，状态栏用 printf 输出。
 *   若 stdout 有缓冲，printf 的内容会在之后某次缓冲区满时才真正
 *   写到“当时”的光标位置，导致状态栏错位、跑到屏幕右边。 */
static void ygq_init_console(void)
{
    char cmd[80];

    /* 关闭 stdout 缓冲：地图用 WriteConsoleOutputW 直接写屏，
     * 状态栏用 printf 输出；若 stdout 有缓冲，printf 的内容会
     * 在之后的某个时刻才真正写到当时的光标位置，导致状态栏错位。
     * 这里直接关闭缓冲，保证每次 printf 都立即输出到正确位置。 */
    setvbuf(stdout, NULL, _IONBF, 0);

    sprintf(cmd, "mode con cols=%d lines=%d >nul 2>&1", CONSOLE_W, CONSOLE_H);
    system(cmd);
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTitleA("掘金者");
    SetConsoleOutputCP(936);  /* 控制台按 GBK 输出中文 */
    SetConsoleCP(936);
    ygq_show_cursor(0);
    ygq_clear_screen();
    ygq_check_window_size();
}

/* ============================================================
 *  地图生成与随机位置
 *  负责画迷宫、判断墙、找随机空地、放道具。
 *  地图只存墙/道具，玩家和僵尸的位置单独存在全局变量/数组里。
 * ============================================================ */

/* ygq_is_wall：判断某个格子能不能走。
 * 参数：x 列、y 行。
 * 返回值：1 表示是墙或越界（不能走），0 表示是空地/道具（可以走）。
 * 说明：玩家和僵尸都只检查 '# '，因此金币、宝石、药水格都可以通过；
 *       越界也当成墙，避免数组越界。 */
static int ygq_is_wall(int x, int y)
{
    if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H)
        return 1;
    return map[y][x] == '#';
}

/* ygq_rand_empty：在地图中随机找一个可用空地。
 * 参数：
 *   px, py   ：输出参数，找到后写入坐标；
 *   min_dist ：与玩家的最小曼哈顿距离（|dx| + |dy|）。
 *              放道具时传 0，放僵尸时传 8，避免一出生就贴脸。
 * 返回值：1 找到，0 尝试 2000 次仍失败。
 *
 * 过滤条件：必须是空格 ' '（不能是墙/已有道具）、不能是玩家脚下、
 *           距离玩家不能小于 min_dist。
 * 说明：86×43 地图空格很多，2000 次基本不会失败；万一失败，
 *       调用方会退回一个默认位置或不放置。 */
static int ygq_rand_empty(int *px, int *py, int min_dist)
{
    int i, x, y;

    for (i = 0; i < 2000; i++) {
        x = 1 + rand() % (MAP_W - 2);
        y = 1 + rand() % (MAP_H - 2);
        if (map[y][x] != ' ')
            continue;
        if (x == pl_x && y == pl_y)
            continue;
        if (abs(x - pl_x) + abs(y - pl_y) < min_dist)
            continue;
        *px = x;
        *py = y;
        return 1;
    }
    return 0;
}

/* ygq_generate_maze：生成一张 86×43 的随机迷宫（原地修改 map）。
 *
 * 算法分三步：
 *   1. 先把所有格子填成墙 '#'；
 *   2. 从 (1,1) 出发做随机深度优先搜索（DFS），每次向随机方向走 2 格，
 *      把中间那格和落点打通。用显式栈 stack_x/stack_y 保存路径，
 *      栈大小 4096 足以覆盖 86×43 的最长路径；无路可走时出栈回溯。
 *   3. 以 12% 概率随机打通一些内墙，制造环路，避免迷宫全是死胡同。
 *
 * 最后把以 (1,1) 为左上角的 3×3 清空，保证玩家一出生不会被墙围死。
 * 说明：本函数只画墙和通道，不放置道具/僵尸；道具和僵尸由
 *       ygq_setup_level / ygq_setup_hell 在迷宫生成后放置。 */
static void ygq_generate_maze(void)
{
    int x, y, i, top;
    int stack_x[4096], stack_y[4096];
    int dirs[4];

    /* 先全部填成墙 */
    for (y = 0; y < MAP_H; y++) {
        for (x = 0; x < MAP_W; x++)
            map[y][x] = '#';
        map[y][MAP_W] = '\0';
    }

    /* 从 (1,1) 开始挖通道，每次走两格 */
    top = 0;
    map[1][1] = ' ';
    stack_x[top] = 1;
    stack_y[top] = 1;
    top++;

    while (top > 0) {
        int cx = stack_x[top - 1];
        int cy = stack_y[top - 1];
        int moved = 0;

        for (i = 0; i < 4; i++)
            dirs[i] = i;
        for (i = 3; i > 0; i--) {
            int j = rand() % (i + 1);
            int t = dirs[i];
            dirs[i] = dirs[j];
            dirs[j] = t;
        }

        for (i = 0; i < 4; i++) {
            int nx = cx;
            int ny = cy;

            if (dirs[i] == 0)
                ny -= 2;
            else if (dirs[i] == 1)
                nx += 2;
            else if (dirs[i] == 2)
                ny += 2;
            else
                nx -= 2;

            if (nx > 0 && nx < MAP_W - 1 && ny > 0 && ny < MAP_H - 1
                && map[ny][nx] == '#') {
                map[(cy + ny) / 2][(cx + nx) / 2] = ' ';
                map[ny][nx] = ' ';
                stack_x[top] = nx;
                stack_y[top] = ny;
                top++;
                moved = 1;
                break;
            }
        }
        if (!moved)
            top--;
    }

    /* 随机打通一些内墙，让迷宫有环路，不至于全是死胡同 */
    for (y = 1; y < MAP_H - 1; y++) {
        for (x = 1; x < MAP_W - 1; x++) {
            if (map[y][x] == '#' && ((x % 2 == 0) != (y % 2 == 0))) {
                if (rand() % 100 < 12)
                    map[y][x] = ' ';
            }
        }
    }

    /* 出生点周围 3x3 清空，作为安全区 */
    for (y = 1; y <= 3; y++)
        for (x = 1; x <= 3; x++)
            map[y][x] = ' ';
}

/* ygq_spawn_item：在随机空地放置一个物品。
 * 参数：kind 为 'G'（金币）、'$'（宝石）或 '+'（药水）。
 * 说明：只写入 map 数组，不立即重绘；调用方随后会统一重绘。
 *       如果地图暂时找不到空地（几乎不会发生），本次就不放，
 *       不会覆盖已有物品或墙。 */
static void ygq_spawn_item(char kind)
{
    int x, y;

    if (ygq_rand_empty(&x, &y, 0))
        map[y][x] = kind;
}

/* ============================================================
 *  画面绘制
 *  地图用 CHAR_INFO 整块刷新；HUD/提示用 printf 按行覆盖。
 *  结算画面、暂停画面属于静态画面，直接清屏后居中写字。
 * ============================================================ */

/* ygq_draw_map：一次性绘制整张地图（墙、道具、僵尸、玩家）。
 *
 * 实现：
 *   1. 先把 map 数组的字符和颜色填进静态 CHAR_INFO 缓冲区 buf；
 *      墙=亮绿，金币=亮黄，宝石=亮青，药水=亮红，空地=白；
 *   2. 再把所有僵尸叠加成红色 'Z'；
 *   3. 最后叠加玩家 'Y'（品红）。玩家受伤后的无敌时间内，
 *      用 frame/3 的奇偶让玩家闪烁，提示“暂时无敌”；
 *   4. 用一次 WriteConsoleOutputW 把整块 86×43 区域写到屏幕左上角。
 *
 * 为什么用 CHAR_INFO：
 *   如果逐格 ygq_cursor + putchar，86×43=3698 格会频繁移动光标、
 *   画面明显闪烁；整块写入只调用一次 API，刷新稳定。
 * 绘制顺序很重要：后叠加的会覆盖先叠加的，所以玩家永远在最上层。 */
static void ygq_draw_map(void)
{
    static CHAR_INFO buf[MAP_H][MAP_W];
    COORD size, pos;
    SMALL_RECT rect;
    int x, y, i;
    WORD attr;
    char ch;

    for (y = 0; y < MAP_H; y++) {
        for (x = 0; x < MAP_W; x++) {
            ch = map[y][x];
            if (ch == '#')
                attr = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            else if (ch == 'G')
                attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            else if (ch == '$')
                attr = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
            else if (ch == '+')
                attr = FOREGROUND_RED | FOREGROUND_INTENSITY;
            else
                attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
            buf[y][x].Char.UnicodeChar = (WCHAR)(unsigned char)ch;
            buf[y][x].Attributes = attr;
        }
    }

    /* 叠加僵尸 */
    for (i = 0; i < enemy_count; i++) {
        if (enemies[i].x >= 0 && enemies[i].x < MAP_W
            && enemies[i].y >= 0 && enemies[i].y < MAP_H) {
            buf[enemies[i].y][enemies[i].x].Char.UnicodeChar = L'Z';
            buf[enemies[i].y][enemies[i].x].Attributes =
                FOREGROUND_RED | FOREGROUND_INTENSITY;
        }
    }

    /* 叠加玩家，受伤后闪烁 */
    if (invincible == 0 || (frame / 3) % 2 == 0) {
        buf[pl_y][pl_x].Char.UnicodeChar = L'Y';
        buf[pl_y][pl_x].Attributes =
            FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    }

    size.X = (SHORT)MAP_W;
    size.Y = (SHORT)MAP_H;
    pos.X = 0;
    pos.Y = 0;
    rect.Left = 0;
    rect.Top = 0;
    rect.Right = (SHORT)(MAP_W - 1);
    rect.Bottom = (SHORT)(MAP_H - 1);
    WriteConsoleOutputW(hOut, &buf[0][0], size, pos, &rect);
}

/* ygq_update_hint：用 BFS 找最近金币，并计算“第一段方向 + 步数”。
 *
 * 输出（全局变量）：
 *   hint_dir   ：第一段方向，0 上 / 1 下 / 2 左 / 3 右，-1 表示无金币；
 *   hint_steps ：这一段连续走多少步。
 *
 * 算法：
 *   1. 从玩家位置开始 BFS，dist 记录到每个格子的最短步数；
 *      from_dir[y][x] 记录“这个格子是从哪个方向走过来的”；
 *   2. BFS 第一次碰到 'G' 时，dist 就是最短总步数；
 *   3. 从金币沿 from_dir 一路回溯到玩家。回溯时统计
 *      “最后一段连续相同的方向”，它正好是玩家出发后的第一段；
 *   4. 只把这一段的方向和步数写进 hint_dir/hint_steps，
 *      所以画面显示“向左 5 步”，而不是总步数 8 步。
 *
 * 例：最短路是 左5步 -> 下3步，则从金币回溯时先连续 3 次“下”，
 *     再连续 5 次“左”；最后一段“左”的 run=5 就是显示结果。
 *
 * 说明：本函数只在 hint_dirty 为 1 时调用（玩家移动/吃到金币后），
 *       避免每帧都做一次 BFS。 */
static void ygq_update_hint(void)
{
    /* qx/qy 是 BFS 队列（最多 MAP_W*MAP_H 个格子）；
     * dist 存最短步数，from_dir 存父方向（0~3，-1 表示起点）。
     * 都声明为 static，避免放在栈上占用过多空间。 */
    static int qx[MAP_W * MAP_H], qy[MAP_W * MAP_H];
    static int dist[MAP_H][MAP_W];
    static int from_dir[MAP_H][MAP_W];
    static const int dx[4] = { 0, 0, -1, 1 };  /* 上 下 左 右 */
    static const int dy[4] = { -1, 1, 0, 0 };
    int head = 0, tail = 0;
    int x, y, i;

    hint_dir = -1;
    hint_steps = 0;

    if (map[pl_y][pl_x] == 'G')
        return;

    for (y = 0; y < MAP_H; y++)
        for (x = 0; x < MAP_W; x++)
            dist[y][x] = -1;

    dist[pl_y][pl_x] = 0;
    from_dir[pl_y][pl_x] = -1;
    qx[tail] = pl_x;
    qy[tail] = pl_y;
    tail++;

    while (head < tail) {
        int cx = qx[head];
        int cy = qy[head];
        head++;

        if (map[cy][cx] == 'G') {
            int last_dir = -1;
            int run = 0;

            /* 从金币往回走到玩家：最后连续相同的方向就是玩家出发的第一段 */
            while (!(cx == pl_x && cy == pl_y)) {
                int d = from_dir[cy][cx];

                if (d == last_dir) {
                    run++;
                } else {
                    last_dir = d;
                    run = 1;
                }
                cx -= dx[d];
                cy -= dy[d];
            }
            hint_dir = last_dir;
            hint_steps = run;
            return;
        }

        for (i = 0; i < 4; i++) {
            int nx = cx + dx[i];
            int ny = cy + dy[i];

            if (nx < 0 || nx >= MAP_W || ny < 0 || ny >= MAP_H)
                continue;
            if (map[ny][nx] == '#')
                continue;
            if (dist[ny][nx] != -1)
                continue;

            dist[ny][nx] = dist[cy][cx] + 1;
            from_dir[ny][nx] = i;
            qx[tail] = nx;
            qy[tail] = ny;
            tail++;
        }
    }
}

/* ygq_draw_hint：绘制第 4 行金币提示（HINT_Y）。
 *
 * 规则：
 *   - 地狱关或提示关闭：整行空白（用 ygq_clear_line 擦除）；
 *   - 提示开启：显示“金币提示：开始    最近金币：<方向> <步数>”；
 *   - 附近没有金币：显示“最近金币：无”。
 * 说明：hint_dirty 为 1 时先调用 ygq_update_hint 重新计算 BFS；
 *       文字用 ygq_safe_width() 补空格，保证整行被覆盖且不换行。 */
static void ygq_draw_hint(void)
{
    char buf[160];
    static const char *dir_name[4] = { "向上", "向下", "向左", "向右" };

    if (hell_mode || !hint_on) {
        ygq_clear_line(HINT_Y);
        return;
    }

    if (hint_dirty) {
        ygq_update_hint();
        hint_dirty = 0;
    }

    if (hint_dir < 0)
        sprintf(buf, "金币提示：开始    最近金币：无");
    else
        sprintf(buf, "金币提示：开始    最近金币：%s %d 步",
                dir_name[hint_dir], hint_steps);

    ygq_color(CYAN);
    ygq_cursor(0, HINT_Y);
    printf("%-*s", ygq_safe_width(), buf);
}

/* ygq_draw_hud：绘制底部 4 行 HUD。
 *
 * 行布局（HUD_Y 起）：
 *   第 1 行：得分 / 关卡（或“地狱”）/ 剩余时间 / 生命；
 *   第 2 行：本关目标进度 / 难度名 / “暂停:P  菜单:Q”；
 *   第 3 行：限时消息（吃到金币、被撞等），无消息时空白；
 *   第 4 行：金币提示，交给 ygq_draw_hint。
 *
 * 说明：每行都用 printf("%-*s", ygq_safe_width(), ...) 补空格，
 *       这样新内容会覆盖旧内容，且不会写到最后一列导致换行。
 *       地图每帧重绘，HUD 也每帧重绘，保证数据实时。 */
static void ygq_draw_hud(void)
{
    char buf[160];

    ygq_color(WHITE);
    ygq_cursor(0, HUD_Y);
    if (hell_mode)
        sprintf(buf, "得分 %-4d 关卡 地狱 时间 %-3d 生命 %d/%d      ",
                score, time_left, hp, hp_max);
    else
        sprintf(buf, "得分 %-4d 关卡 %-2d 时间 %-3d 生命 %d/%d      ",
                score, level, time_left, hp, hp_max);
    printf("%-*s", ygq_safe_width(), buf);

    ygq_cursor(0, HUD2_Y);
    sprintf(buf, "目标 %d/%d   难度:%s   暂停:P   菜单:Q          ",
            level_score, goal, difficulty_name);
    printf("%-*s", ygq_safe_width(), buf);

    if (message[0] != '\0')
        ygq_color(CYAN);
    else
        ygq_color(WHITE);
    ygq_cursor(0, MSG_Y);
    printf("%-*s", ygq_safe_width(), message);

    ygq_draw_hint();
}

/* ygq_draw_all：重绘整个游戏画面（地图 + HUD）。
 * 说明：每帧游戏循环都会调用；先画地图再画 HUD，
 *       HUD 在地图下方，不会互相覆盖。 */
static void ygq_draw_all(void)
{
    ygq_draw_map();
    ygq_draw_hud();
}

/* ygq_draw_pause：暂停时在地图中央盖一块提示框。
 *
 * 位置计算：
 *   box_w = 30，box_x = (MAP_W - box_w) / 2，
 *   box_y = MAP_H / 2 - 1；
 *   文字用 strlen 计算宽度并相对 MAP_W 居中。
 * 说明：先 ygq_clear_rect 擦掉 30×3 的区域，再写“暂 停”和
 *       “按 P 键继续游戏”，这样提示不会被地图的 '#' 挡住。
 *       恢复游戏时由 ygq_draw_all 重绘地图，提示自然消失。 */
static void ygq_draw_pause(void)
{
    const char *title = "暂 停";
    const char *tip = "按 P 键继续游戏";
    int box_w = 30;
    int box_x = (MAP_W - box_w) / 2;
    int box_y = MAP_H / 2 - 1;

    ygq_clear_rect(box_x, box_y, box_w, 3);
    ygq_text((MAP_W - (int)strlen(title)) / 2, box_y, YELLOW, title);
    ygq_text((MAP_W - (int)strlen(tip)) / 2, box_y + 1, WHITE, tip);
}

/* ygq_draw_level_clear：普通关通过画面。
 *
 * 显示内容：
 *   - “关 卡 通 过 ！”；
 *   - 第几关、难度、本关得分 / 目标；
 *   - 剩余时间奖励（1 秒 = 1 分）；
 *   - 当前总分；
 *   - 按空格进入下一关，或（第 3 关后）查看通关结果。
 * 说明：所有行都加 UI_TOP（6），让画面在 48 行窗口中垂直居中；
 *       本画面只负责显示，按键处理在 ygq_play_game 的
 *       ST_LEVEL_CLEAR 分支。 */
static void ygq_draw_level_clear(void)
{
    char buf[160];

    ygq_clear_screen();
    ygq_text_center(UI_TOP + 7, YELLOW, "关 卡 通 过 ！");

    sprintf(buf, "第 %d 关（%s）得分：%d / %d",
            level, difficulty_name, level_score, goal);
    ygq_text_center(UI_TOP + 10, WHITE, buf);

    sprintf(buf, "剩余时间奖励：+%d 分", level_bonus);
    ygq_text_center(UI_TOP + 11, CYAN, buf);

    sprintf(buf, "当前总分：%d 分", score);
    ygq_text_center(UI_TOP + 13, GREEN, buf);

    if (level >= TOTAL_LEVELS) {
        ygq_text_center(UI_TOP + 16, MAGENTA, "按空格键查看通关结果");
    } else {
        sprintf(buf, "按空格键进入第 %d 关", level + 1);
        ygq_text_center(UI_TOP + 16, MAGENTA, buf);
    }
}

/* ygq_draw_game_over：失败画面（普通关和地狱关共用）。
 *
 * 显示内容：
 *   - “游 戏 结 束”；
 *   - 到达关卡：普通关显示“第 N 关（难度）”，地狱显示“地狱关卡”；
 *   - 最终得分；
 *   - 普通关：R 重新开始 / Q 回主菜单；
 *     地狱关：R 再挑战地狱 / Q 回主菜单。
 * 触发时机：时间到 或 生命降到 0，由 ygq_end_game 调用。
 * 说明：所有行加 UI_TOP 居中；本画面不处理按键，按键在
 *       ygq_play_game 的 ST_GAME_OVER 分支处理。 */
static void ygq_draw_game_over(void)
{
    char buf[160];

    ygq_clear_screen();
    ygq_text_center(UI_TOP + 7, RED, "游 戏 结 束");

    if (hell_mode) {
        ygq_text_center(UI_TOP + 10, WHITE, "到达关卡：地狱关卡");
    } else {
        sprintf(buf, "到达关卡：第 %d 关（%s）", level, difficulty_name);
        ygq_text_center(UI_TOP + 10, WHITE, buf);
    }

    sprintf(buf, "最终得分：%d 分", score);
    ygq_text_center(UI_TOP + 11, YELLOW, buf);

    if (hell_mode)
        ygq_text_center(UI_TOP + 14, CYAN, "按 R 再挑战地狱");
    else
        ygq_text_center(UI_TOP + 14, CYAN, "按 R 重新开始");

    ygq_text_center(UI_TOP + 15, CYAN, "按 Q 返回主菜单");
}

/* ygq_draw_victory：普通三关全部通过后的通关画面。
 *
 * 显示内容：
 *   - “恭 喜 通 关 ！”；
 *   - “你闯过了全部 3 关”；
 *   - 最终得分；
 *   - R 再玩一次 / Q 回主菜单。
 * 说明：由 ygq_play_game 的 ST_LEVEL_CLEAR 分支在最后一关按空格后
 *       设置 state = ST_VICTORY 并调用；行号同样加 UI_TOP 居中。 */
static void ygq_draw_victory(void)
{
    char buf[160];

    ygq_clear_screen();
    ygq_text_center(UI_TOP + 6, YELLOW, "恭 喜 通 关 ！");
    ygq_text_center(UI_TOP + 9, GREEN, "你闯过了全部 3 关");

    sprintf(buf, "最终得分：%d 分", score);
    ygq_text_center(UI_TOP + 11, YELLOW, buf);

    ygq_text_center(UI_TOP + 14, CYAN, "按 R 再玩一次");
    ygq_text_center(UI_TOP + 15, CYAN, "按 Q 返回主菜单");
}

/* ygq_draw_hell_clear：地狱关通过后的独立通关画面。
 *
 * 显示内容：
 *   - “地 狱 通 关 ！”；
 *   - 最终得分；
 *   - “你征服了 32 只僵尸的围追堵截”；
 *   - R 再挑战地狱 / Q 回主菜单。
 * 说明：和普通通关画面分开，是为了强调隐藏关的独立结算；
 *       所有行加 UI_TOP 居中，32 与 HELL_ZOMBIES 保持一致。 */
static void ygq_draw_hell_clear(void)
{
    char buf[160];

    ygq_clear_screen();
    ygq_text_center(UI_TOP + 7, RED, "地 狱 通 关 ！");

    sprintf(buf, "最终得分：%d 分", score);
    ygq_text_center(UI_TOP + 10, YELLOW, buf);

    ygq_text_center(UI_TOP + 13, CYAN, "你征服了 32 只僵尸的围追堵截");
    ygq_text_center(UI_TOP + 16, MAGENTA, "按 R 再挑战地狱");
    ygq_text_center(UI_TOP + 17, MAGENTA, "按 Q 返回主菜单");
}

/* ============================================================
 *  开始界面与隐藏秘籍
 *  秘籍只在开始菜单有效；解锁状态仅保留到程序退出。
 * ============================================================ */

/* ygq_konami_key：菜单秘籍状态机，序列为
 *   W W S S A D A D B A
 * 参数：
 *   key ：本次按下的键；
 *   idx ：指向当前已匹配到第几个字符的计数器（调用方保存）。
 * 返回值：1 表示完整输入了秘籍，0 表示还没输完。
 *
 * 匹配规则：
 *   - 字母统一转成大写，所以大小写都行；
 *   - 如果当前键等于 seq[*idx]，计数器 +1；到 10 个时归零并返回 1；
 *   - 如果按错：
 *       * 按的是第一个字符 W，则把它当作新序列的第 1 个（idx=1）；
 *       * 否则整个序列作废（idx=0）。
 * 说明：只在开始菜单调用；游戏中和结算画面按键不会触发秘籍。 */
static int ygq_konami_key(int key, int *idx)
{
    static const int seq[10] = {
        'W', 'W', 'S', 'S', 'A', 'D', 'A', 'D', 'B', 'A'
    };
    int k = key;

    /* 小写转大写，实现大小写均可 */
    if (k >= 'a' && k <= 'z')
        k -= 32;

    if (k == seq[*idx]) {
        (*idx)++;
        if (*idx >= 10) {
            *idx = 0;
            return 1;
        }
    } else {
        /* 按错时：如果按的是 W，可以当作重新开始；否则清零 */
        *idx = (k == seq[0]) ? 1 : 0;
    }
    return 0;
}

/* ygq_draw_start_screen：绘制开始菜单。
 *
 * 画面内容（从上到下）：
 *   - 两条分隔线夹着标题“掘 金 者”；
 *   - 图例：Y 玩家、G 金币、$ 宝石、+ 药水、Z 僵尸；
 *   - 操作说明：移动、暂停、返回菜单、目标、三关难度；
 *   - 菜单项：1 开始游戏 / 2 退出游戏；
 *   - 若已解锁地狱：显示“隐藏关卡：地狱已解锁”“按 0 进入”；
 *   - 底部提示当前可输入的键。
 * 说明：分隔线用 ygq_separator 动态生成，宽度随窗口变化；
 *       每次秘籍解锁或从游戏返回菜单都会重新调用本函数。 */
static void ygq_draw_start_screen(void)
{
    ygq_clear_screen();

    ygq_separator(2);
    ygq_text_center(3, YELLOW, "掘 金 者");
    ygq_separator(4);

    ygq_text(4, 6, MAGENTA, "Y  你（玩家）");
    ygq_text(4, 7, YELLOW, "G  金币        吃掉 +1 分");
    ygq_text(4, 8, CYAN, "$  宝石        吃掉 +3 分");
    ygq_text(4, 9, RED, "+  药水        恢复 +1 点生命（满血时 +1 分）");
    ygq_text(4, 10, RED, "Z  僵尸        碰到 -1 点生命，快躲开！");

    ygq_separator(11);

    ygq_text(4, 13, WHITE, "移动：方向键 或 W A S D");
    ygq_text(4, 14, WHITE, "暂停：P          返回菜单：Q");
    ygq_text(4, 15, WHITE, "目标：90 秒内收集到目标分数即可过关，共 3 关");
    ygq_text(4, 16, WHITE, "难度：第 1 关简单 / 第 2 关中等 / 第 3 关困难");

    ygq_separator(17);

    ygq_text(4, 19, CYAN, "1. 开始游戏");
    ygq_text(4, 20, CYAN, "2. 退出游戏");

    if (hell_unlocked) {
        ygq_text(4, 22, RED, "隐藏关卡：地狱已解锁");
        ygq_text(4, 23, YELLOW, "按 0 进入地狱关卡");
        ygq_text(4, 25, WHITE, "请输入 1、2 或 0：");
    } else {
        ygq_text(4, 25, WHITE, "请输入 1 或 2：");
    }
}

/* ygq_start_screen：开始菜单的按键循环。
 * 返回值：
 *   1 = 开始普通三关；
 *   2 = 进入隐藏地狱关（仅解锁后）；
 *   0 = 退出程序。
 *
 * 流程：
 *   1. 先清空一次输入缓冲，避免菜单显示前的按键影响判断；
 *   2. 绘制菜单；
 *   3. 每 50ms 读一次键：
 *      - 先交给 ygq_konami_key 检查秘籍，解锁后立即重绘菜单；
 *      - '1' 返回 1；'2'/'Q'/Esc 返回 0；
 *      - 已解锁时 '0' 返回 2；
 *      - 其他键忽略。
 * 说明：菜单里不响应暂停/移动，方向键也不会做任何事。 */
static int ygq_start_screen(void)
{
    int key, konami_idx = 0;

    ygq_flush_input();
    ygq_draw_start_screen();

    while (1) {
        key = ygq_read_key();
        if (key != 0) {
            if (ygq_konami_key(key, &konami_idx)) {
                hell_unlocked = 1;
                ygq_flush_input();
                ygq_draw_start_screen();
            }

            if (key == '1')
                return 1;
            if (key == '2' || key == 'q' || key == 'Q' || key == 27)
                return 0;
            if (key == '0' && hell_unlocked)
                return 2;
        }
        Sleep(50);
    }
}

/* ============================================================
 *  关卡与游戏逻辑
 *  包括关卡初始化、拾取、僵尸 AI、受伤、每帧更新、玩家移动、
 *  胜负结算以及一局游戏的完整主循环。
 * ============================================================ */

/* ygq_set_message：设置 HUD 第 3 行的限时消息。
 * 参数：s 要显示的字符串（会被截断到 message 缓冲区大小）。
 * 说明：msg_timer 设为 40 帧，约 2 秒后由 ygq_update 自动清空；
 *       再次调用会覆盖上一条消息并重新计时。 */
static void ygq_set_message(const char *s)
{
    strncpy(message, s, sizeof(message) - 1);
    message[sizeof(message) - 1] = '\0';
    msg_timer = 40;  /* 约 2 秒 */
}

/* ygq_place_enemy：为第 idx 只僵尸找一个出生位置。
 * 参数：
 *   idx      ：僵尸在 enemies 数组中的下标；
 *   min_dist ：与玩家的最小曼哈顿距离（通常传 8）。
 *
 * 过程：
 *   1. 调用 ygq_rand_empty 找空格；
 *   2. 再与前面已经放好的 0 ~ idx-1 只僵尸比较，
 *      如果坐标重复就重新找，最多尝试 500 次；
 *   3. 成功后写入坐标，并给 tick 一个 0 ~ interval-1 的随机初值，
 *      让僵尸不会在同一帧整齐地一起移动。
 *
 * 说明：如果一次都没找到（几乎不会发生），就使用右下角默认坐标；
 *       如果找到但始终与前面的僵尸重叠，则使用最后一次找到的坐标。
 *       两种情况都能保证数组里没有未初始化的僵尸。 */
static void ygq_place_enemy(int idx, int min_dist)
{
    int tries, j;
    int x = MAP_W - 2, y = MAP_H - 2;

    for (tries = 0; tries < 500; tries++) {
        if (!ygq_rand_empty(&x, &y, min_dist))
            continue;
        for (j = 0; j < idx; j++) {
            if (enemies[j].x == x && enemies[j].y == y)
                break;
        }
        if (j >= idx)
            break;
    }

    enemies[idx].x = x;
    enemies[idx].y = y;
    enemies[idx].tick = rand() % enemy_interval;
}

/* ygq_setup_level：初始化当前普通关（第 level 关）。
 *
 * 依次完成：
 *   1. 从 level_cfg[level-1] 读取本关难度名、僵尸数、速度、目标分；
 *   2. 生成新迷宫；
 *   3. 玩家回到左上角 (1,1)，本关分数清零，生命回满，
 *      给 30 帧（1.5 秒）出生保护；
 *   4. 按 GOLD_COUNT / GEM_COUNT / POTION_COUNT 放道具；
 *   5. 按本关僵尸数放僵尸，要求离玩家至少 8 格且互不重叠；
 *   6. 设置开局提示并把 hint_dirty 置 1，下一帧重算金币提示。
 * 说明：总分 score 不在这里清零，由 ygq_new_game 负责；
 *       所以进入下一关时总分是累计的。 */
static void ygq_setup_level(void)
{
    const LevelCfg *cfg = &level_cfg[level - 1];
    int i;

    hell_mode = 0;
    hp_max = MAX_HP;
    difficulty_name = cfg->name;

    ygq_generate_maze();

    pl_x = 1;
    pl_y = 1;
    level_score = 0;
    goal = cfg->goal;
    time_left = 90;
    hp = hp_max;
    invincible = 30;  /* 出生保护 1.5 秒 */
    frame = 0;

    enemy_count = cfg->zombies;
    enemy_interval = cfg->interval;
    enemy_random_pct = cfg->random_pct;

    /* 放置金币、宝石、药水 */
    for (i = 0; i < GOLD_COUNT; i++)
        ygq_spawn_item('G');
    for (i = 0; i < GEM_COUNT; i++)
        ygq_spawn_item('$');
    for (i = 0; i < POTION_COUNT; i++)
        ygq_spawn_item('+');

    /* 放置僵尸，要求离玩家至少 8 格 */
    for (i = 0; i < enemy_count; i++)
        ygq_place_enemy(i, 8);

    sprintf(message, "第 %d 关开始！难度：%s  目标 %d 分",
            level, difficulty_name, goal);
    msg_timer = 40;
    hint_dirty = 1;
}

/* ygq_setup_hell：初始化隐藏地狱关。
 *
 * 与普通关的区别：
 *   - hell_mode = 1，HUD 显示“地狱”，第 4 行金币提示被屏蔽；
 *   - 生命 3、时间 75 秒、目标 40 分；
 *   - 32 只僵尸、250ms 移动一次、5% 随机乱走；
 *   - 12 金币 / 4 宝石 / 2 药水；
 *   - 总分 score 在这里清零，地狱关分数不继承普通关。
 * 其余流程（生成迷宫、放玩家、出生保护、放道具、放僵尸）与普通关一致。 */
static void ygq_setup_hell(void)
{
    int i;

    hell_mode = 1;
    hp_max = HELL_HP;
    difficulty_name = "地狱";

    ygq_generate_maze();

    pl_x = 1;
    pl_y = 1;
    score = 0;
    level_score = 0;
    goal = HELL_GOAL;
    time_left = HELL_TIME;
    hp = hp_max;
    invincible = 30;
    frame = 0;

    enemy_count = HELL_ZOMBIES;
    enemy_interval = HELL_INTERVAL;
    enemy_random_pct = HELL_RANDOM_PCT;

    for (i = 0; i < HELL_GOLD; i++)
        ygq_spawn_item('G');
    for (i = 0; i < HELL_GEM; i++)
        ygq_spawn_item('$');
    for (i = 0; i < HELL_POTION; i++)
        ygq_spawn_item('+');

    for (i = 0; i < enemy_count; i++)
        ygq_place_enemy(i, 8);

    sprintf(message, "进入隐藏关卡：地狱关卡！目标 %d 分", goal);
    msg_timer = 40;
    hint_dirty = 1;
}

/* ygq_end_level：普通关通过结算。
 * 行为：
 *   - 状态切到 ST_LEVEL_CLEAR；
 *   - level_bonus = 剩余秒数，并加到总分（1 秒 = 1 分）；
 *   - 立即绘制过关画面。
 * 说明：本关得分 level_score 不在这里清零，进入下一关时由
 *       ygq_setup_level 清零。 */
static void ygq_end_level(void)
{
    state = ST_LEVEL_CLEAR;
    level_bonus = time_left;
    score += level_bonus;
    ygq_draw_level_clear();
}

/* ygq_end_game：失败结算（普通关和地狱关共用）。
 * 触发条件：
 *   - 时间倒计时减到 0；
 *   - 生命降到 0。
 * 行为：状态切到 ST_GAME_OVER，并绘制失败画面。
 * 说明：普通关/地狱关的失败文字由 ygq_draw_game_over 内部
 *       根据 hell_mode 区分。 */
static void ygq_end_game(void)
{
    state = ST_GAME_OVER;
    ygq_draw_game_over();
}

/* ygq_end_hell_clear：地狱关通过结算。
 * 行为：状态切到 ST_HELL_CLEAR，绘制地狱独立通关画面。
 * 说明：与普通关的 ygq_end_level 分开，因为地狱关不需要
 *       “进入下一关”，也不累计普通关的剩余时间奖励。 */
static void ygq_end_hell_clear(void)
{
    state = ST_HELL_CLEAR;
    ygq_draw_hell_clear();
}

/* ygq_check_pickup：检查玩家脚下有没有道具，有就吃掉并补一个新的。
 *
 * 处理规则：
 *   'G' 金币 ：总分 +1，本关分 +1，补一个金币；
 *   '$' 宝石 ：总分 +3，本关分 +3，补一个宝石；
 *   '+' 药水 ：未满血时生命 +1；满血时改为总分/本关分 +1，
 *               然后补一个药水。
 * 其他字符（墙/空地）直接返回。
 *
 * 每次吃到东西都会把 hint_dirty 置 1，让金币提示重新 BFS。
 * 最后检查 level_score >= goal：
 *   - 地狱模式 -> ygq_end_hell_clear；
 *   - 普通模式 -> ygq_end_level。
 * 说明：本函数在 ygq_update 中每帧调用一次，位置在僵尸移动之前。 */
static void ygq_check_pickup(void)
{
    char ch = map[pl_y][pl_x];

    if (ch == ' ' || ch == '#' || ch == '\0')
        return;
    map[pl_y][pl_x] = ' ';
    hint_dirty = 1;

    if (ch == 'G') {
        score += 1;
        level_score += 1;
        ygq_set_message("吃到金币  +1 分");
        ygq_spawn_item('G');
    } else if (ch == '$') {
        score += 3;
        level_score += 3;
        ygq_set_message("吃到宝石  +3 分");
        ygq_spawn_item('$');
    } else if (ch == '+') {
        if (hp < hp_max) {
            hp++;
            ygq_set_message("喝下药水  生命 +1");
        } else {
            score += 1;
            level_score += 1;
            ygq_set_message("生命已满  药水换成 +1 分");
        }
        ygq_spawn_item('+');
    }

    if (level_score >= goal) {
        if (hell_mode)
            ygq_end_hell_clear();
        else
            ygq_end_level();
    }
}

/* ygq_move_enemy：让一只僵尸向玩家方向移动一步。
 * 参数：e 指向要移动的僵尸。
 *
 * 行为分两种：
 *   1. 以 enemy_random_pct 的概率随机走一格（方向随机），
 *      让僵尸看起来不那么“机械”；
 *   2. 否则贪心追踪：
 *      - 先比较横向和纵向距离，距离大的轴优先追；
 *      - 把首选方向、次选方向依次放进候选数组；
 *      - 再追加左右上下四个备用方向；
 *      - 从 cand[0] 开始找第一个不是墙的方向走；
 *      - 如果四面都是墙，就原地不动。
 * 说明：僵尸不检查其他僵尸/玩家所在格，所以移动后可能重叠，
 *       这是正常现象；碰到玩家的判定在 ygq_update 里统一做。 */
static void ygq_move_enemy(Enemy *e)
{
    int dx = pl_x - e->x;          /* 玩家相对僵尸的横向距离 */
    int dy = pl_y - e->y;          /* 玩家相对僵尸的纵向距离 */
    int sx = (dx > 0) - (dx < 0);  /* 横向追踪方向：-1/0/1 */
    int sy = (dy > 0) - (dy < 0);  /* 纵向追踪方向：-1/0/1 */
    int adx = abs(dx);
    int ady = abs(dy);
    int cand[6][2];                /* 候选方向：最多 2 个追踪 + 4 个备用 */
    int n = 0, i;

    /* 按关卡配置的概率随机走一步，避免僵尸死板地直线追击 */
    if (rand() % 100 < enemy_random_pct) {
        int d = rand() % 4;
        int rnd[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
        int nx = e->x + rnd[d][0];
        int ny = e->y + rnd[d][1];

        if (!ygq_is_wall(nx, ny)) {
            e->x = nx;
            e->y = ny;
            return;
        }
    }

    /* 优先沿距离大的方向追，再沿另一个方向追 */
    if (adx >= ady) {
        if (sx != 0) {
            cand[n][0] = sx;
            cand[n][1] = 0;
            n++;
        }
        if (sy != 0) {
            cand[n][0] = 0;
            cand[n][1] = sy;
            n++;
        }
    } else {
        if (sy != 0) {
            cand[n][0] = 0;
            cand[n][1] = sy;
            n++;
        }
        if (sx != 0) {
            cand[n][0] = sx;
            cand[n][1] = 0;
            n++;
        }
    }

    /* 备用方向 */
    cand[n][0] = 1;  cand[n][1] = 0;  n++;
    cand[n][0] = -1; cand[n][1] = 0;  n++;
    cand[n][0] = 0;  cand[n][1] = 1;  n++;
    cand[n][0] = 0;  cand[n][1] = -1; n++;

    for (i = 0; i < n; i++) {
        int nx = e->x + cand[i][0];
        int ny = e->y + cand[i][1];

        if (!ygq_is_wall(nx, ny)) {
            e->x = nx;
            e->y = ny;
            return;
        }
    }
}

/* ygq_hurt_player：玩家被第 idx 只僵尸撞到时的处理。
 *
 * 处理顺序：
 *   1. 生命 -1，设置 30 帧（1.5 秒）无敌时间；
 *   2. 显示“被僵尸撞到！生命 -1”；
 *   3. 如果生命 <= 0，置 0 并调用 ygq_end_game 结束本局；
 *   4. 否则把这只僵尸随机挪到离玩家至少 8 格的位置，tick 清零，
 *      给玩家逃跑机会，避免解除无敌后立刻又被同一只撞。
 * 说明：无敌期间 ygq_update 不会再次判定碰撞。 */
static void ygq_hurt_player(int idx)
{
    int nx, ny;

    hp--;
    invincible = 30;  /* 1.5 秒无敌时间 */
    ygq_set_message("被僵尸撞到！生命 -1");

    if (hp <= 0) {
        hp = 0;
        ygq_end_game();
        return;
    }

    /* 把撞人的僵尸挪远，给玩家逃跑的机会 */
    if (ygq_rand_empty(&nx, &ny, 8)) {
        enemies[idx].x = nx;
        enemies[idx].y = ny;
        enemies[idx].tick = 0;
    }
}

/* ygq_update：游戏每帧（约 50ms）更新一次。
 *
 * 更新顺序（顺序会影响手感和判定）：
 *   1. frame++，无敌帧、消息计时各减 1，消息到 0 时清空；
 *   2. 每 20 帧（约 1 秒）倒计时一次；时间到 0 就失败返回；
 *   3. 检查玩家是否吃到道具；若因此过关/失败，state 已改变，
 *      立即返回，不再移动僵尸；
 *   4. 每只僵尸 tick++，达到 enemy_interval 时移动一格；
 *   5. 无敌帧为 0 时，检查是否有僵尸与玩家同格；
 *      有则调用 ygq_hurt_player。
 * 说明：本函数由 ygq_play_game 在 ST_PLAYING 状态下每帧调用；
 *       暂停状态下不会调用，所以暂停时计时和僵尸都会停住。 */
static void ygq_update(void)
{
    int i;

    frame++;

    if (invincible > 0)
        invincible--;

    if (msg_timer > 0) {
        msg_timer--;
        if (msg_timer == 0)
            message[0] = '\0';
    }

    /* 每 20 帧（约 1 秒）倒计时一次 */
    if (frame % 20 == 0) {
        if (time_left > 0)
            time_left--;
        if (time_left <= 0) {
            time_left = 0;
            ygq_end_game();
            return;
        }
    }

    ygq_check_pickup();
    if (state != ST_PLAYING)
        return;

    for (i = 0; i < enemy_count; i++) {
        enemies[i].tick++;
        if (enemies[i].tick >= enemy_interval) {
            enemies[i].tick = 0;
            ygq_move_enemy(&enemies[i]);
        }
    }

    if (invincible == 0) {
        for (i = 0; i < enemy_count; i++) {
            if (enemies[i].x == pl_x && enemies[i].y == pl_y) {
                ygq_hurt_player(i);
                break;
            }
        }
    }
}

/* ygq_try_move_player：根据按键尝试移动玩家一格。
 * 参数：key 为方向键键值或 W/A/S/D 字符。
 *
 * 行为：
 *   - W / 上箭头 -> dy = -1；S / 下箭头 -> dy = +1；
 *   - A / 左箭头 -> dx = -1；D / 右箭头 -> dx = +1；
 *   - 其他键：dx/dy 都是 0，直接返回；
 *   - 目标格不是墙才更新 pl_x/pl_y，并置 hint_dirty = 1，
 *     让金币提示在下一帧重新计算。
 * 说明：玩家移动后不会立刻拾取道具，拾取统一由
 *       ygq_update -> ygq_check_pickup 处理。 */
static void ygq_try_move_player(int key)
{
    int dx = 0, dy = 0;

    if (key == KEY_UP || key == 'w' || key == 'W')
        dy = -1;
    else if (key == KEY_DOWN || key == 's' || key == 'S')
        dy = 1;
    else if (key == KEY_LEFT || key == 'a' || key == 'A')
        dx = -1;
    else if (key == KEY_RIGHT || key == 'd' || key == 'D')
        dx = 1;

    if (dx == 0 && dy == 0)
        return;

    if (!ygq_is_wall(pl_x + dx, pl_y + dy)) {
        pl_x += dx;
        pl_y += dy;
        hint_dirty = 1;
    }
}

/* ygq_new_game：开始新的普通一局。
 * 行为：总分清零、关卡设为第 1 关、初始化关卡、状态设为进行中。
 * 说明：从失败/通关画面按 R 会调用它；普通关之间的“下一关”
 *       不调用它，而是直接 level++ 后 ygq_setup_level，所以总分保留。 */
static void ygq_new_game(void)
{
    score = 0;
    level = 1;
    ygq_setup_level();
    state = ST_PLAYING;
}

/* ygq_start_hell：进入隐藏地狱关。
 * 行为：
 *   1. 清屏并显示 1.5 秒的进入提示：
 *      “进入隐藏关卡：地狱关卡”
 *      “32 只僵尸 / 3 点生命 / 75 秒 / 目标 40 分”；
 *   2. 调用 ygq_setup_hell 初始化；
 *   3. 状态设为进行中并重绘。
 * 说明：从菜单按 0、或在地狱结算画面按 R 都会走这里；
 *       提示行加 UI_TOP 居中，数字与 HELL_* 宏保持一致。 */
static void ygq_start_hell(void)
{
    ygq_clear_screen();
    ygq_text_center(UI_TOP + 13, RED, "进入隐藏关卡：地狱关卡");
    ygq_text_center(UI_TOP + 15, WHITE, "32 只僵尸 / 3 点生命 / 75 秒 / 目标 40 分");
    Sleep(1500);
    ygq_setup_hell();
    state = ST_PLAYING;
    ygq_draw_all();
}

/* ygq_play_game：一局游戏的完整主循环。
 * 参数：start_hell 非 0 表示从地狱关开始，0 表示普通三关。
 * 返回：按 Q（或暂停时按 Q）返回主菜单，由 main() 继续显示菜单。
 *
 * 状态机：
 *   ST_PLAYING    ：读键 -> Q 回菜单 / P 暂停 / 000 切换提示 /
 *                   移动 -> ygq_update -> 重绘；
 *   ST_PAUSED     ：只响应 P 继续、Q 回菜单；
 *   ST_LEVEL_CLEAR：空格（或回车）进入下一关，最后一关后进入通关画面；
 *   ST_VICTORY / ST_GAME_OVER / ST_HELL_CLEAR：
 *                   R 重开（普通/地狱按 hell_mode 区分），Q 回菜单。
 * 每轮循环末尾 Sleep(50)，即游戏逻辑约 20 帧/秒。
 *
 * 说明：000 提示开关只在地狱以外生效；zero_count 记录连续按 0 的次数，
 *       按其他键会清零，所以必须“连按 3 个 0”才切换。 */
static void ygq_play_game(int start_hell)
{
    int key;

    if (start_hell)
        ygq_start_hell();   /* 地狱关：先显示 1.5 秒进入提示 */
    else {
        ygq_new_game();     /* 普通关：从第 1 关开始 */
        ygq_draw_all();
    }

    while (1) {
        if (state == ST_PLAYING) {
            key = ygq_read_key();

            if (key == 'q' || key == 'Q')
                return;
            if (key == 'p' || key == 'P') {
                state = ST_PAUSED;
                ygq_draw_pause();
                Sleep(200);
                continue;
            }

            /* 连按 3 个 0 开关金币提示；地狱关完全不响应 */
            if (key == '0') {
                if (!hell_mode) {
                    zero_count++;
                    if (zero_count >= 3) {
                        zero_count = 0;
                        hint_on = !hint_on;
                        hint_dirty = 1;
                    }
                }
            } else if (key != 0) {
                zero_count = 0;
            }

            if (key != 0)
                ygq_try_move_player(key);

            if (state == ST_PLAYING) {
                ygq_update();
                if (state == ST_PLAYING)
                    ygq_draw_all();
            }
        } else if (state == ST_PAUSED) {
            key = ygq_read_key();
            if (key == 'p' || key == 'P') {
                state = ST_PLAYING;
                ygq_draw_all();
            } else if (key == 'q' || key == 'Q') {
                return;
            }
        } else if (state == ST_LEVEL_CLEAR) {
            key = ygq_read_key();
            if (key == ' ' || key == 13) {
                if (level >= TOTAL_LEVELS) {
                    state = ST_VICTORY;
                    ygq_draw_victory();
                } else {
                    level++;
                    ygq_setup_level();
                    state = ST_PLAYING;
                    ygq_draw_all();
                }
            }
        } else if (state == ST_VICTORY || state == ST_GAME_OVER
                   || state == ST_HELL_CLEAR) {
            key = ygq_read_key();
            if (key == 'r' || key == 'R') {
                if (hell_mode)
                    ygq_start_hell();
                else {
                    ygq_new_game();
                    ygq_draw_all();
                }
            } else if (key == 'q' || key == 'Q') {
                return;
            }
        }
        Sleep(50);
    }
}

/* ---------------- 主函数 ---------------- */

/* main：程序入口。
 * 流程：
 *   1. 初始化控制台（窗口、标题、GBK、光标、尺寸检查）；
 *   2. 用当前时间初始化随机数种子，保证每局迷宫不同；
 *   3. 循环显示开始菜单：
 *      - 返回 0  -> 退出循环；
 *      - 返回 1  -> 普通三关；
 *      - 返回 2  -> 地狱关；
 *      一局结束后清屏，回到菜单；
 *   4. 退出前恢复光标、白色文字并清屏。
 * 说明：开始菜单的“地狱解锁”状态只在本次运行内有效，
 *       程序退出后不会写文件，重新运行需再次输入秘籍。 */
int main(void)
{
    int choice;

    ygq_init_console();
    srand((unsigned)time(NULL));

    while ((choice = ygq_start_screen()) != 0) {
        ygq_clear_screen();
        ygq_play_game(choice == 2 ? 1 : 0);
        ygq_clear_screen();
    }

    ygq_show_cursor(1);
    ygq_color(WHITE);
    ygq_clear_screen();
    return 0;
}

