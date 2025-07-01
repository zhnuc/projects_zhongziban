#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "text_operations.h"

#define BUFFER_SIZE 4096  // 缓冲区大小
#define CLEAR_SCREEN "\033[2J\033[H"  // ANSI 转义序列：清屏并将光标移到左上角

// 光标控制转义序列
#define CURSOR_LEFT "\033[D"
#define CURSOR_RIGHT "\033[C"
#define CURSOR_UP "\033[A"
#define CURSOR_DOWN "\033[B"
#define CURSOR_HOME "\033[H"      // 光标移动到行首
#define CURSOR_END "\033[4~"       // 光标移动到行尾
#define CURSOR_PGUP "\033[5~"     // 向上翻页
#define CURSOR_PGDN "\033[6~"     // 向下翻页
#define BACKSPACE_SEQ "\033[D \033[D"  // 左移+空格+左移，模拟退格

// 特殊键值 - 使用远离ASCII范围的值
#define KEY_ESC     27      // ESC键的ASCII值
#define KEY_LEFT    1000
#define KEY_RIGHT   1001
#define KEY_UP      1002
#define KEY_DOWN    1003
#define KEY_DEL     1004
#define KEY_HOME    1005
#define KEY_END     1006
#define KEY_PGUP    1007
#define KEY_PGDN    1008

// 读取按键，处理多字节序列（如箭头键）
int read_key() {
    int c;
    char seq[3];
    
    // 读取第一个字节
    if (read(STDIN_FILENO, &c, 1) != 1) return -1;
    
    // 检查是否是转义序列开始
    if (c == KEY_ESC) {
        // 尝试读取更多字符，但如果没有更多字符，就返回ESC
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return KEY_ESC;
        
        if (seq[0] == '[') {
            // 读取第三个字节
            if (read(STDIN_FILENO, &seq[1], 1) != 1) return KEY_ESC;
            
            // 根据第三个字节判断是哪个特殊键
            switch (seq[1]) {
                case 'A': return KEY_UP;
                case 'B': return KEY_DOWN;
                case 'C': return KEY_RIGHT;
                case 'D': return KEY_LEFT;
                case 'H': return KEY_HOME;
                case '4': 
                    if (read(STDIN_FILENO, &seq[2], 1) != 1) return KEY_ESC; 
                    if (seq[2] == '~') return KEY_END; 
                    break;
                case '5': // 读取第四个字节，确认是 Page Up
                    if (read(STDIN_FILENO, &seq[2], 1) != 1) return KEY_ESC;
                    if (seq[2] == '~') return KEY_PGUP;
                    break;
                case '6': // 读取第四个字节，确认是 Page Down
                    if (read(STDIN_FILENO, &seq[2], 1) != 1) return KEY_ESC;
                    if (seq[2] == '~') return KEY_PGDN;
                    break;
            }
        }
        
        // 无法识别的转义序列
        return KEY_ESC;
    } else {
        return c;  // 普通字符
    }
}

// 在缓冲区中添加转义序列的函数

// 在缓冲区中添加转义序列
void add_escape_sequence(char *buffer, int *pos, const char *seq) {
    int len = strlen(seq);
    if (*pos + len < BUFFER_SIZE) {
        memcpy(buffer + *pos, seq, len);
        *pos += len;
        buffer[*pos] = '\0';
    }
}

void text_op(char c) {
    // 创建文本缓冲区
    char *buffer = malloc(BUFFER_SIZE);
    if (!buffer) {
        perror("内存分配失败");
        return;
    }
    
    // 初始化缓冲区
    buffer[0] = '\0';
    int buffer_pos = 0;
    
    // 显示初始屏幕
    write(STDOUT_FILENO, CLEAR_SCREEN, strlen(CLEAR_SCREEN));
    char *welcome = "编辑器启动，按 q 退出\r\n";
    write(STDOUT_FILENO, welcome, strlen(welcome));

    // 主循环
    while (1) {
        // 读取按键
        int key = read_key();
        
        if (key == 'q') break;  // 退出条件
        
        // 处理特殊按键
        if (key == KEY_LEFT) {
            add_escape_sequence(buffer, &buffer_pos, CURSOR_LEFT);
        } 
        else if (key == KEY_RIGHT) {
            add_escape_sequence(buffer, &buffer_pos, CURSOR_RIGHT);
        } 
        else if (key == KEY_UP) {
            add_escape_sequence(buffer, &buffer_pos, CURSOR_UP);
        } 
        else if (key == KEY_DOWN) {
            add_escape_sequence(buffer, &buffer_pos, CURSOR_DOWN);
        } 
        else if (key == KEY_HOME) {
            add_escape_sequence(buffer, &buffer_pos, CURSOR_HOME);
        }
        else if (key == KEY_END) {
            add_escape_sequence(buffer, &buffer_pos, CURSOR_END);
        }
        else if (key == KEY_PGUP) {
            add_escape_sequence(buffer, &buffer_pos, CURSOR_PGUP);
        }
        else if (key == KEY_PGDN) {
            add_escape_sequence(buffer, &buffer_pos, CURSOR_PGDN);
        }
        else if (key == '\r' || key == '\n') {
            if (buffer_pos + 2 < BUFFER_SIZE) {
                buffer[buffer_pos++] = '\r';
                buffer[buffer_pos++] = '\n';
                buffer[buffer_pos] = '\0';
            }
        } 
        else if (key == 127 || key == 8) { // 退格键
            if (buffer_pos > 0) {
                // 使用左移+空格+左移序列模拟退格
                add_escape_sequence(buffer, &buffer_pos, BACKSPACE_SEQ);
            }
        } 
        else if (key >= 32 && key < 127) { // 可打印ASCII字符
            if (buffer_pos + 1 < BUFFER_SIZE) {
                buffer[buffer_pos++] = key;
                buffer[buffer_pos] = '\0';
            }
        }
        
        // 清屏并重新显示内容
        write(STDOUT_FILENO, CLEAR_SCREEN, strlen(CLEAR_SCREEN));
        write(STDOUT_FILENO, welcome, strlen(welcome));
        
        // 输出缓冲区内容
        if (buffer_pos > 0) {
            write(STDOUT_FILENO, buffer, buffer_pos);
        }
        

    }
    
    // 清理
    free(buffer);
}