/* ansi.c - utility functions for making things pretty
 *
 * Copyright (C) 2007 by Alex Osborne <bobofdoom@gmail.com>
 *
 * Licensed under the GPL v2 or later, see the file LICENSE.
 */

#include "ansi.h"
#include <termios.h>

/**
 * Determine the deminesions of the console.
 */
struct winsize win = { 0, 0, 0, 0 };
void refresh_winsize(void)
{
	int ret = ioctl(fileno(stdin), TIOCGWINSZ, &win);
	if (ret == -1) {
		win.ws_col = 80;
		win.ws_row = 25;
	}

	/* limit to 60 characters for testing */
	if (win.ws_col > 60 || !win.ws_col) 
		win.ws_col = 60;
}

inline void clear_screen(void)
{
	printf(CLEAR);
}

/**
 * Draw a fancy horizontal rule.
 */
void draw_hr(void)
{
	int i;
	printf(" ");
	for (i=1; i < win.ws_col -1; i++) {
		if (i % 2)
			printf("-");
		else
			printf("=");
	}
	printf("\n");
}

/**
 * Calculate the length of s excluding ANSI escape codes.
 */
int strlen_skipansi(const char *s)
{
	int len=0;

	while (*s) {
		if (*s == '\033') {
			while(!isalpha(*s) && *s) s++;
		}

		if (!*s) break;

		if (isgraph(*s) || *s == ' ') len++;
		s++;
	}

	return len;
}

/**
 * Print a formatted string centered.
 */
int printfc(const char *fmt, ...)
{
	va_list ap;
	char buf[256];
	int len, pad;

	va_start(ap, fmt);
	len = vsnprintf (buf, sizeof(buf), fmt, ap);
	va_end(ap);

	pad = (win.ws_col - strlen_skipansi(buf)) / 2;
	while (pad-- > 0)
		printf(" ");
	
	printf(buf);
	return len;
}

/**
 * Print a gentoo-like ebegin message.
 */
int ebegin(const char *fmt, ...)
{
	va_list ap;
	int len;

	printf(BOLDON GREENF " * " RESET);
	va_start(ap, fmt);
	len = vprintf(fmt, ap);
	va_end(ap);
	return len;
}

/**
 * Print a gentoo-like einfo message.
 */
int einfo(const char *fmt, ...)
{
	va_list ap;
	int len;

	printf(BOLDON GREENF " * " RESET);
	va_start(ap, fmt);
	len = vprintf(fmt, ap);
	va_end(ap);
	printf("\n");
	return len;
}

/**
 * Print a gentoo-like ewarn message.
 */
int ewarn(const char *fmt, ...)
{
	va_list ap;
	int len;

	printf(BOLDON YELLOWF " * " RESET);
	va_start(ap, fmt);
	len = vprintf(fmt, ap);
	va_end(ap);
	printf("\n");
	return len;
}

/**
 * Print a gentoo-like eerror message.
 */
int eerror(const char *fmt, ...)
{
	va_list ap;
	int len;

	printf(BOLDON REDF " * " RESET);
	va_start(ap, fmt);
	len = vprintf(fmt, ap);
	va_end(ap);
	printf("\n");
	return len;
}

/**
 * Gentoo-like eend message: [ ok ] or [ !! ]
 */
int eend(int err)
{
	printf("\033[%dG", win.ws_col - 6);

	if (err) {
		printf(BOLDON BLUEF "[ " REDF "!!" BLUEF " ]" RESET "\n");
	} else {
		printf(BOLDON BLUEF "[ " GREENF "ok" BLUEF " ]" RESET "\n");
	}
	return err;
}
	
/**
 * Read a raw character, do not wait for return, do not echo it
 * and do not collect $200.
 */
int getch(void)
{
	int c;
	struct termios old, new;
	tcgetattr(STDIN_FILENO, &old);
	new = old;
	new.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &new);
	c = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &old);
	return c;
}

