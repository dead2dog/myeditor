#include<ctype.h>
#include<errno.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<termios.h>

struct termios orig_termios;

void die(const char *s)//err handing
{
    perror(s);
    exit(1);
}

void disbaleRawMode()//恢复shell
{
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
        die("tcsetattr");  
}


void enableRawMode() //原始模式
{
    if(tcgetattr(STDIN_FILENO,&orig_termios) == -1) die("tcgetattr");
    atexit(disbaleRawMode); //注册终止函数

    struct termios raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO|ICANON|ISIG|IEXTEN);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    if(tcsetattr(STDIN_FILENO,TCSAFLUSH,&raw)) die("tcsetattr");
}

int main()
{
    enableRawMode();
    while(1)
    {
        char c ='\0';
        if(read(STDIN_FILENO,&c,1) == -1 && errno != EAGAIN) die("read");
        if(iscntrl(c))
        {
            printf("%d\r\n",c);
        }else{
            printf("%d ('%c')\r\n",c,c);
        }
        if(c == 'q') break;
    }
    return 0;
}