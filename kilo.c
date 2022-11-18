#include<ctype.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<termios.h>

struct termios orig_termios;

void disbaleRawMode()//恢复shell
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}


void enableRawMode() //原始模式
{
    tcgetattr(STDIN_FILENO,&orig_termios);
    atexit(disbaleRawMode); //注册终止函数

    struct termios raw = orig_termios;
    raw.c_iflag &= ~(IXON|ICRNL);
    raw.c_lflag &= ~(ECHO|ICANON|ISIG|IEXTEN);
    tcsetattr(STDIN_FILENO,TCSAFLUSH,&raw);
}

int main()
{
    enableRawMode();
    char c;
    while(read(STDIN_FILENO,&c,1) == 1 && c != 'q')
        if(iscntrl(c))
        {
            printf("%d\n",c);
        }else{
            printf("%d ('%c')\n",c,c);
        }
    return 0;
}