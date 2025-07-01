#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct termios orig_termios;

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);
    
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main() {
    enableRawMode();
    
    printf("按键测试程序 - 按 q 退出\r\n");
    printf("------------------------\r\n");
    
    unsigned char c;
    unsigned char seq[8]; // 存储转义序列
    int seq_pos;
    
    while (1) {
        read(STDIN_FILENO, &c, 1);
        
        if (c == 'q') break;  // 退出条件
        
        // 清屏并移动光标到左上角
        printf("\033[2J\033[H");
        printf("按键测试程序 - 按 q 退出\r\n");
        printf("------------------------\r\n");
        
        if (c == 27) {  // ESC
            printf("检测到转义序列: ESC (27, 0x1B)\r\n");
            
            // 读取并显示转义序列
            seq_pos = 0;
            seq[seq_pos++] = c;
            
            // 尝试读取更多字符
            fd_set readfds;
            struct timeval tv;
            
            while (1) {
                FD_ZERO(&readfds);
                FD_SET(STDIN_FILENO, &readfds);
                tv.tv_sec = 0;
                tv.tv_usec = 100000;  // 100ms超时
                
                if (select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv) > 0) {
                    if (read(STDIN_FILENO, &c, 1) == 1) {
                        seq[seq_pos++] = c;
                        seq[seq_pos] = '\0';
                    } else {
                        break;
                    }
                } else {
                    break;  // 超时，没有更多字符
                }
                
                if (seq_pos >= 7) break;  // 防止缓冲区溢出
            }
            
            // 显示完整的转义序列
            printf("完整的转义序列 (%d 字节): ", seq_pos);
            for (int i = 0; i < seq_pos; i++) {
                printf("%d ", seq[i]);
            }
            printf("\r\n");
            
            printf("对应的转义码: ");
            for (int i = 0; i < seq_pos; i++) {
                printf("\\x%02X", seq[i]);
            }
            printf("\r\n");
            
            // 尝试识别特殊键
            if (seq_pos >= 3 && seq[0] == 27 && seq[1] == '[') {
                switch (seq[2]) {
                    case 'A': printf("键识别: 上箭头键\r\n"); break;
                    case 'B': printf("键识别: 下箭头键\r\n"); break;
                    case 'C': printf("键识别: 右箭头键\r\n"); break;
                    case 'D': printf("键识别: 左箭头键\r\n"); break;
                    case 'H': printf("键识别: Home键\r\n"); break;
                    case 'F': printf("键识别: End键\r\n"); break;
                    case '4':
                        if (seq_pos >= 4 && seq[3] == '~')
                            printf("键识别: End键 (\\x1B[4~)\r\n");
                        break;
                    case '5':
                        if (seq_pos >= 4 && seq[3] == '~')
                            printf("键识别: Page Up键\r\n");
                        break;
                    case '6':
                        if (seq_pos >= 4 && seq[3] == '~')
                            printf("键识别: Page Down键\r\n");
                        break;
                    default:
                        printf("键识别: 未知特殊键\r\n");
                }
            } else if (seq_pos >= 3 && seq[0] == 27 && seq[1] == 'O') {
                switch (seq[2]) {
                    case 'H': printf("键识别: Home键 (\\x1BOH)\r\n"); break;
                    case 'F': printf("键识别: End键 (\\x1BOF)\r\n"); break;
                    default:
                        printf("键识别: 未知特殊键\r\n");
                }
            }
        } else {
            // 显示普通字符
            printf("按键: '%c' (ASCII: %d, HEX: 0x%02X)\r\n", 
                   c >= 32 && c < 127 ? c : ' ', c, c);
        }
    }
    
    return 0;
}
