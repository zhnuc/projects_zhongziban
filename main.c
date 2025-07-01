#include <stdio.h>
#include "editor.h"
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include "text_operations.h"

int main(){
    
    enableRawMode();
    
    char c = 0;
    printf("编辑器已启动，按 q 退出\r\n");
    
    text_op(c);
    
    disableRawMode();
    return 0;    
}
