#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include "editor.h"
#include <errno.h>

struct termios orig_termios;

void enableRawMode() {
    struct termios raw;
    
    // 保存原始终端设置
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        perror("tcgetattr");
        return;
    }
    
    raw = orig_termios; // 从原始设置开始修改
    
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
    // VMIN = 1: read() 至少读取 1 个字节才返回
    // VTIME = 0: 不设置超时，read() 会一直等待直到读取到至少一个字节
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    // 应用新的终端设置
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr");
        return;
    }
}

void disableRawMode() {
    // 直接恢复之前保存的原始终端设置
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
        perror("tcsetattr");
    }
}
