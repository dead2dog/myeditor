#include<ctype.h>
#include<errno.h>
#include<stdio.h>
#include<stdlib.h>
#include <sys/ioctl.h>
#include<unistd.h>
#include<termios.h>


#define CTRL_KEY(k) ((k) & 0x1f) //使ctrl能继续使用

/*** data ***/
struct editorConfig {
    int screenrows;
    int screencols;
    struct termios orig_termios;//保留terminal原始状态
};
struct editorConfig E;

/*** terminal ***/
void die(const char *s)//err handing
{

    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    perror(s);
    exit(1);
}

char editorReadKey() {
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) die("read");
    }
    return c;
}

void disbaleRawMode()//恢复terminal状态
{
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
        die("tcsetattr");  
}

void enableRawMode() //原始模式
{
    if(tcgetattr(STDIN_FILENO,&E.orig_termios) == -1) die("tcgetattr");
    atexit(disbaleRawMode); //注册终止函数

    struct termios raw = E.orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO|ICANON|ISIG|IEXTEN);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    if(tcsetattr(STDIN_FILENO,TCSAFLUSH,&raw)) die("tcsetattr");
}


int getWindowSize(int *rows, int *cols) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        return -1;
    } else {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}
/*** input ***/

void editorProcessKeypress() {
    char c = editorReadKey();
    switch (c) {
        case CTRL_KEY('q'):
        write(STDOUT_FILENO, "\x1b[2J", 4); //清屏
        exit(0);
        break;
    }
}

/*** output ***/
void editorDrawRows() {
    int y;
    for (y = 0; y < E.screenrows; y++) {
        write(STDOUT_FILENO, "~\r\n", 3);
    }
}

void editorRefreshScreen() {
    write(STDOUT_FILENO, "\x1b[2J", 4); 
    //J -> erase in display ; 0j 1j 2j
    //\x1 = \27 == [esc] ->escape character followed by [;
    write(STDOUT_FILENO, "\x1b[H", 3);
    //H -> Reposition the cursor
    editorDrawRows();
    write(STDOUT_FILENO, "\x1b[H", 3);
}



/*** init ***/
void initEditor() {
    if (getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");
}

int main()
{
    enableRawMode();
    initEditor();
    while(1)
    {
        editorRefreshScreen();
        editorProcessKeypress();
    }
    return 0;
}