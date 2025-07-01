#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "text_operations.h"

#define BUFFER_SIZE 4096  // 缓冲区大小
#define CLEAR_SCREEN "\033[2J\033[H"  // ANSI 转义序列：清屏并将光标移到左上角

// 特殊键值
#define KEY_ESC     27  // ESC键的ASCII值
#define KEY_LEFT    1000
#define KEY_RIGHT   1001
#define KEY_UP      1002
#define KEY_DOWN    1003
#define KEY_DEL     1004
#define KEY_HOME    1005
#define KEY_END     1006

// 读取按键，处理多字节序列（如箭头键）
int read_key() {
    int c;
    char seq[3];
    
    if (read(STDIN_FILENO, &c, 1) != 1) return -1;
    
    if (c == KEY_ESC) {  // 转义序列开始
        // 读取后续字符
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return KEY_ESC;
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return KEY_ESC;
        
        if (seq[0] == '[') {
            switch (seq[1]) {
                case 'A': return KEY_UP;
                case 'B': return KEY_DOWN;
                case 'C': return KEY_RIGHT;
                case 'D': return KEY_LEFT;
                case 'H': return KEY_HOME;
                case 'F': return KEY_END;
            }
        }
        
        // 无法识别的转义序列
        return KEY_ESC;
    } else {
        return c;  // 普通字符
    }
}

// 移动光标到指定位置 (1,1是左上角)
void move_cursor_to(int x, int y) {
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "\033[%d;%dH", y, x);
    write(STDOUT_FILENO, buf, len);
}

// 光标左移
void cursor_left() {
    write(STDOUT_FILENO, "\033[D", 3);
}

// 光标右移
void cursor_right() {
    write(STDOUT_FILENO, "\033[C", 3);
}

// 光标上移
void cursor_up() {
    write(STDOUT_FILENO, "\033[A", 3);
}

// 光标下移
void cursor_down() {
    write(STDOUT_FILENO, "\033[B", 3);
}

void text_op(char c){
    // 创建文本缓冲区
    char *buffer = malloc(BUFFER_SIZE);
    if (!buffer) {
        perror("内存分配失败");
        return;
    }
    
    // 初始化缓冲区和光标位置
    buffer[0] = '\0';
    int buffer_pos = 0;
    int cursor_x = 0;  // 光标水平位置
    int cursor_y = 1;  // 光标垂直位置（从1开始，因为有欢迎消息）
    
    // 显示初始屏幕
    write(STDOUT_FILENO, CLEAR_SCREEN, strlen(CLEAR_SCREEN));
    char *welcome = "编辑器启动，按 q 退出\r\n";
    write(STDOUT_FILENO, welcome, strlen(welcome));

    while (1) {
        // 将光标移动到当前位置
        move_cursor_to(cursor_x + 1, cursor_y + 1);
        
        // 读取按键
        int key = read_key();
        
        if (key == 'q') break;  // 退出条件
        
        // 处理特殊按键
        switch(key) {
            case KEY_LEFT:
                if (cursor_x > 0) cursor_x--;
                break;
                
            case KEY_RIGHT:
                cursor_x++;
                break;
                
            case KEY_UP:
                if (cursor_y > 0) cursor_y--;
                break;
                
            case KEY_DOWN:
                cursor_y++;
                break;
                
            case '\r':
            case '\n':
                // 添加换行到缓冲区
                if (buffer_pos + 2 < BUFFER_SIZE) {
                    buffer[buffer_pos++] = '\r';
                    buffer[buffer_pos++] = '\n';
                    buffer[buffer_pos] = '\0';
                    cursor_x = 0;
                    cursor_y++;
                }
                break;
                
            case 127: // 退格键
            case 8:
                if (buffer_pos > 0) {
                    buffer_pos--;
                    buffer[buffer_pos] = '\0';
                    if (cursor_x > 0) {
                        cursor_x--;
                    } else if (cursor_y > 0) {
                        cursor_y--;
                        // 这里简化处理，实际应该计算上一行的长度
                        cursor_x = 10; // 假设值
                    }
                }
                break;
                
            default:
                if (key >= 32 && key < 127) {  // 可打印ASCII字符
                    if (buffer_pos + 1 < BUFFER_SIZE) {
                        buffer[buffer_pos++] = key;
                        buffer[buffer_pos] = '\0';
                        cursor_x++;
                    }
                }
                break;
        }
        
        // 清屏并重新显示整个缓冲区内容
        write(STDOUT_FILENO, CLEAR_SCREEN, strlen(CLEAR_SCREEN));
        write(STDOUT_FILENO, welcome, strlen(welcome));
        
        // 输出缓冲区内容
        if (buffer_pos > 0) {
            write(STDOUT_FILENO, buffer, buffer_pos);
        }
        
        // 显示当前按键和光标位置信息（调试用）
        char info[64];
        int info_len = snprintf(info, sizeof(info), "\r\n[按键: %d, 光标: (%d,%d)]\r\n", 
                               key, cursor_x, cursor_y);
        write(STDOUT_FILENO, info, info_len);
    }
    
    // 清理
    free(buffer);
    return;
}

