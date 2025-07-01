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

// 调试模式开关
#define DEBUG_MODE 1

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
        
        // 初始化用于调试的信息字符串
        char debug_info[128];
        
        // 处理特殊按键
        if (key == KEY_LEFT) {
            snprintf(debug_info, sizeof(debug_info), "左箭头键");
            add_escape_sequence(buffer, &buffer_pos, CURSOR_LEFT);
        } 
        else if (key == KEY_RIGHT) {
            snprintf(debug_info, sizeof(debug_info), "右箭头键");
            add_escape_sequence(buffer, &buffer_pos, CURSOR_RIGHT);
        } 
        else if (key == KEY_UP) {
            snprintf(debug_info, sizeof(debug_info), "上箭头键");
            add_escape_sequence(buffer, &buffer_pos, CURSOR_UP);
        } 
        else if (key == KEY_DOWN) {
            snprintf(debug_info, sizeof(debug_info), "下箭头键");
            add_escape_sequence(buffer, &buffer_pos, CURSOR_DOWN);
        } 
        else if (key == '\r' || key == '\n') {
            snprintf(debug_info, sizeof(debug_info), "回车键");
            if (buffer_pos + 2 < BUFFER_SIZE) {
                buffer[buffer_pos++] = '\r';
                buffer[buffer_pos++] = '\n';
                buffer[buffer_pos] = '\0';
            }
        } 
        else if (key == 127 || key == 8) { // 退格键
            snprintf(debug_info, sizeof(debug_info), "退格键");
            if (buffer_pos > 0) {
                // 使用左移+空格+左移序列模拟退格
                add_escape_sequence(buffer, &buffer_pos, BACKSPACE_SEQ);
            }
        } 
        else if (key >= 32 && key < 127) { // 可打印ASCII字符
            snprintf(debug_info, sizeof(debug_info), "字符: %c", key);
            if (buffer_pos + 1 < BUFFER_SIZE) {
                buffer[buffer_pos++] = key;
                buffer[buffer_pos] = '\0';
            }
        } 
        else {
            snprintf(debug_info, sizeof(debug_info), "未知按键: %d", key);
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