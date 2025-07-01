#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include "editor.h"

void enableRawMode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    
    // 修改 raw.c_lflag 来禁用终端的一些本地模式
    // 禁用 ECHO: 关闭回显
    // 禁用 ICANON: 关闭规范模式 (行缓冲), 使得输入按字符处理
    // 禁用 IEXTEN: 禁用扩展输入处理
    // 禁用 ISIG: 禁用信号生成 (例如 Ctrl+C 不再生成 SIGINT 信号)
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);

    // 修改 raw.c_iflag 来禁用某些输入处理
    // 禁用 BRKINT, ICRNL, INPCK, ISTRIP, IXON 等标志
    raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);

    // 修改 raw.c_oflag 来禁用输出处理 (比如自动转换换行符)
    raw.c_oflag &= ~(OPOST);

    // 修改 raw.c_cflag" 设置字符大小为 8 位 (通常为 CS8)
    raw.c_cflag |= (CS8);

    // 设置控制字符
    // VMIN = 0: read() 至少读取 0 个字节
    // VTIME = 1000: read() 超时时间为 1000ms
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1000;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disableRawMode() {
    struct termios orig_termios;
    tcgetattr(STDIN_FILENO, &orig_termios);
    
    // 恢复原始终端设置
    orig_termios.c_lflag |= (ECHO | ICANON | ISIG | IEXTEN);
    orig_termios.c_iflag |= (IXON | ICRNL | BRKINT | INPCK | ISTRIP);
    orig_termios.c_oflag |= (OPOST);
    orig_termios.c_cflag |= (CS8);
    
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

int main() {
    char c;
    enableRawMode();
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q') {
        if (c == '\r' | c == '\n') {
            write(STDOUT_FILENO, "\r\n", 2);
        }
        write(STDOUT_FILENO, &c, 1);
        //显示按键的ASCII值，包括控制字符
        printf("Key pressed: %d\n", c); // 打印按键的ASCII值
        fflush(stdout); // 确保输出立即刷新

    }
    disableRawMode();
    return 0;
}