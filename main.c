#include <stdio.h>
#include "editor.h"
#include <unistd.h>
#include <termios.h>
#include <errno.h>


int main(){
    
    enableRawMode();
    
    char c;
    printf("编辑器已启动，按 q 退出\r\n");
    
    while (1) {
        c = 0;
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EINTR) {
            perror("read");
            return 1;
        }
        
        if (c == 'q') break;
        
        if (c == '\r' || c == '\n') {
            // 处理回车键
            write(STDOUT_FILENO, "\r\n", 2);
        } else if (c > 0) {
            // 显示按键信息（使用 write 避免行缓冲问题）
            char buf[32];
            int len = snprintf(buf, sizeof(buf), " %c (ASCII: %d)", c, c);
            write(STDOUT_FILENO, buf, len);
        }
    }
    
    disableRawMode();
    return 0;    
}
