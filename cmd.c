// Originally written by ezhikov
// See http://hackndev.com/node/188

#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define OK 0
#define ERROR (-1)
#define CMD_SIZE 256

#define UP 65
#define DOWN 66
#define RIGHT 67
#define LEFT 68
#define ENTER 10
#define BACKSPACE 'f'
#define EXIT ("ex")

#define DUP_IN 10
#define DUP_OUT 11

#define LLEN 6

static struct termios term_state;
static const char* const syms = "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXWZ'$|><&*\\/ !.,{}[];:()#@+-?\"_";

char* get_sel(int num, int pos) {
	static const char *esc_l = "\033[0;37;40m";
	static const char *esc_sl = "\033[0;30;47m";
	static const char *esc_r = "\033[0;0;0m";
	/*static char *ret[strlen(esc_l)*2 + strlen(esc_sl) + strlen(esc_r) + LLEN + 2];*/
	static char ret[10 * 2 + 10 + 8 + LLEN + 2];
	char sm[LLEN + 1], tmp;
	int sl = strlen(syms);
	int i;
	sm[LLEN] = 0;
	for(i = 0; i < LLEN; i++)
		sm[i] = syms[num * LLEN % sl + i];

	tmp = sm[pos];
	sm[pos] = 0;
	sprintf(ret, "%s|%s%s%c%s%s%s",esc_l, sm , esc_sl, tmp, esc_l, &sm[pos + 1], esc_r);
	return ret;
}

int save_term() {
	if(1 == isatty(0) && 0 <= tcgetattr(0, &term_state)) {
		return OK;
	} else {
		return ERROR;
	}
}

void restore_term() {
	tcsetattr(DUP_IN, TCSAFLUSH, &term_state);
}

int set_term() {
	struct termios ts;
	memcpy(&ts, &term_state, sizeof(ts));
	ts.c_lflag &= ~ICANON;
	ts.c_lflag &= ~ECHO;
	ts.c_cc[VMIN] = 1;
	ts.c_cc[VTIME] = 0;
	return (0 == tcsetattr(0, TCSAFLUSH, &ts)) ? OK : ERROR;
}

void drop(int line, int pos) {
	char* str = get_sel(line, pos);
	write(DUP_OUT, str, strlen(str));

}

void clean_drop(int line, int pos) {
	char buff[LLEN];
	int i;
	for(i = 0; i <= LLEN; i++)
		buff[i] = '\b';
	write(DUP_OUT, buff, LLEN + 1);
	drop(line, pos);
}

void clean_show(char c) {
	char buff[LLEN + 2];
	int i;
	for(i = 0; i <= LLEN;i++)
		buff[i] = '\b';
	buff[LLEN + 1] = c;
	write(DUP_OUT, buff, LLEN + 2);
}

int execute(char* cmd) {
	int c_s;
	pid_t c_pid;
	restore_term();
	c_pid = fork();
	if(0 == c_pid) {
		execl(getenv("SHELL"), "sh", "-c", cmd , 0);
		exit(0);
	}
	wait(&c_s);

	set_term();
	return OK;
}

int main_loop() {
	char dest[CMD_SIZE];
	char btn;
	unsigned int cmd_pos = 0;
	int line=0, pos=0;
	int table_l = strlen(syms);
	int last_enter = 0;
	int i;

	drop(line, pos);
	do{
		read(DUP_IN, &btn, 1);
		switch(btn) {
			case LEFT:
				pos = (pos > 0) ? (pos - 1) : (LLEN - 1);
				last_enter = 0;
				clean_drop(line, pos);
				break;
			case RIGHT:
				pos = (pos + 1) % LLEN;
				last_enter = 0;
				clean_drop(line, pos);
				break;
			case UP:
				line = (line > 0) ? (line - 1) : (table_l / LLEN - 1);
				last_enter = 0;
				clean_drop(line, pos);
				break;
			case DOWN:
				line = (line + 1) % (table_l / LLEN);
				last_enter = 0;
				clean_drop(line, pos);
				break;
			case ENTER:
				if (last_enter == 1) {
					if(0 == strcmp(dest, EXIT)) {
						write(DUP_OUT, "\n", 1);
						return OK;
					} else {
						int i;
						dest[ cmd_pos++ ] = '\n';
						dest[cmd_pos]=0;
						write(DUP_OUT, "\n", 1);
#if 0

// WTF? There actually IS space character. It comes earlier that _
// There are problems with it when typing something like 'modprobe pxa27x_udc' :)))
// -- slapin
						for(i = 0; dest[i] != 0; i++)
							if(dest[i]=='_')
								dest[i] = ' ';
#endif
						execute(dest);
					}
					last_enter = 0;
					cmd_pos = 0;
					drop(line, pos);
				} else {
					last_enter = 1;
					clean_show(syms[line * LLEN + pos]);
					dest[cmd_pos++] = syms[line * LLEN + pos];
					dest[cmd_pos] = 0;
					drop(line, pos);
				}
				break;
			case BACKSPACE:
				if(cmd_pos > 0) {
					write(DUP_OUT, "\r", 1);
					for(i=0; i<cmd_pos + LLEN + 2; i++)
						write(DUP_OUT, " ", 1);
					write(DUP_OUT, "\r", 1);
					dest[--cmd_pos] = 0;
					write(DUP_OUT, dest, cmd_pos);
					drop(line, pos);
				}
			default:
				{}
		}
	}while(1);
	return OK;
}

int main(int argc, char** args) {
	if(OK != save_term()) {
		printf("%s\n","descriptor 0 is not terminal");
		return ERROR;
	}
	atexit(restore_term);
	if(OK != set_term()) {
		printf("%s\n","unable to set terminal");
		return ERROR;
	}
	dup2(0, DUP_IN);
	dup2(1, DUP_OUT);
	return main_loop();
}
