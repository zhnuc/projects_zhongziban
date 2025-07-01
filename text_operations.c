#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "text_operations.h"

#define BUFFER_SIZE 4096  // 缓冲区大小
#define CLEAR_SCREEN "\033[2J\033[H"  // ANSI 转义序列：清屏并将光标移到左上角

void text_op(char c){
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
    write(STDOUT_FILENO, "编辑器启动，按 q 退出\r\n", 22);

    while (1) {
        c = 0;

        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EINTR) {
            perror("read");
            free(buffer);
            return;
        }
        
        if (c == 'q') break;  // 退出条件
        
        // 处理特殊按键
        if (c == '\r' || c == '\n') {
            // 添加换行到缓冲区
            if (buffer_pos + 2 < BUFFER_SIZE) {
                buffer[buffer_pos++] = '\r';
                buffer[buffer_pos++] = '\n';
                buffer[buffer_pos] = '\0';
            }
        } 
        // 处理退格键
        else if (c == 127 || c == 8) {
            if (buffer_pos > 0) {
                buffer_pos--;
                buffer[buffer_pos] = '\0';
            }
        }
        // 处理可打印字符
        else if (c >= 32 && c < 127) {  // 可打印ASCII字符
            if (buffer_pos + 1 < BUFFER_SIZE) {
                buffer[buffer_pos++] = c;
                buffer[buffer_pos] = '\0';
            }
        }
        
        // 清屏并重新显示整个缓冲区内容
        write(STDOUT_FILENO, CLEAR_SCREEN, strlen(CLEAR_SCREEN));
        write(STDOUT_FILENO, "编辑器启动，按 q 退出\r\n", 22);
        
        // 输出缓冲区内容
        if (buffer_pos > 0) {
            write(STDOUT_FILENO, buffer, buffer_pos);
        }
        
        // 显示当前按键信息（调试用）
        char info[64];
        int info_len = snprintf(info, sizeof(info), "\r\n[按键: %c (%d)]\r\n", c, c);
        write(STDOUT_FILENO, info, info_len);
    }
    
    // 清理
    free(buffer);
    return;
}

