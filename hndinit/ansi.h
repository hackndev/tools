#ifndef _ANSI_H_
#define _ANSI_H_

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <termios.h>

#include <sys/mount.h>
#include <sys/ioctl.h>


/* ANSI colour codes */
#define BLACKF	"\033[30m"
#define REDF	"\033[31m"
#define GREENF	"\033[32m"
#define YELLOWF "\033[33m"
#define BLUEF	"\033[34m"
#define PURPLEF "\033[35m"
#define CYANF	"\033[36m"
#define WHITEF	"\033[37m"

#define BLACKB	"\033[40m"
#define REDB	"\033[41m"
#define GREENB	"\033[42m"
#define YELLOWB "\033[43m"
#define BLUEB	"\033[44m"
#define PURPLEB "\033[45m"
#define CYANB	"\033[46m"
#define WHITEB	"\033[47m"

#define BOLDON	"\033[1m"
#define BOLDOFF "\033[22m"
#define ITALICSON	"\033[3m"
#define ITALICSOFF	"\033[23m"
#define ULON	"\033[4m"
#define ULOFF	"\033[24m"
#define INVON	"\033[7m"
#define INVOFF	"\033[27m"

#define RESET	"\033[0m"

#define CLEAR	"\033[H\033[2J"

extern struct winsize win;
void refresh_winsize(void);
void clear_screen(void);
void draw_hr(void);
int strlen_skipansi(const char *s);
int printfc(const char *fmt, ...);
int ebegin(const char *fmt, ...);
int einfo(const char *fmt, ...);
int ewarn(const char *fmt, ...);
int eerror(const char *fmt, ...);
int eend(int err);
int getch(void);
#endif

