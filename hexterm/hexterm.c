/*******************************************************************************
 *
 *  hexterm - stupid hex i/o terminal
 *
 *  (C) 2007 by Vladimir "Farcaller" Pouzanov <farcaller@gmail.com>
 *  Parts of code from bcm-tool by Daniel Ribeiro
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 
 *  as published by the Free Software Foundation
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>

#define MAX_REQUEST	1024
#define TRUE		1
#define FALSE		0

/*** parser and console i/o ***/

char parse_char(const char *c)
{
	long int i;
	if(strlen(c)>2)
		return 0;
	i = strtol(c, NULL, 16);
	if(i>255)
		return 0;
	return i;
}

char * parse_line(char *line, int *len)
{
	char *p, *s;
	char b[4];
	char *buff, *bp;
	char byte;
	int done = FALSE;
	int cnt = 0;

	s = p = line;
	buff = malloc(MAX_REQUEST);
	bp = buff;
	while (*p != '\0' && !done) {
		while (*p != '\0' && *p != ' ')
			p++;
		if (*p == '\0') {
			done = TRUE;
		} else {
			*p = '\0';
			p++;
		}
		strncpy(b, s, 3);
		b[3] = '\0';
		byte = parse_char(b);
		*bp = byte;
		bp++;
		cnt++;
		s = p;
	}
	*len = cnt;
	return buff;
}

/*** serial i/o ***/

int set_baud(int fd, int baud)
{
	struct termios t;
	char buf[8];

	printf("< set baud: %d\n", baud);
	tcgetattr(fd, &t);
	switch (baud) {
		case 115200:
			//send_cmd(fd, CMD_SET_BAUD, BAUD115200, 2);
        		cfsetospeed(&t, B115200);
			break;
		case 460800:
			//send_cmd(fd, CMD_SET_BAUD, BAUD460800, 2);
			cfsetospeed(&t, B460800);
			break;
		case 921600:
			//send_cmd(fd, CMD_SET_BAUD, BAUD921600, 2);
        		cfsetospeed(&t, B921600);
			break;
		default:
			printf ("invalid baud rate!\n");
			return(-1);
	}
	return tcsetattr(fd, TCSANOW, &t);
}


void send_data(int fd, char *bytes, int len)
{
	if (len == 0) {
		printf("==> ...\n");
		return;
	}
	printf("==[%d][", len);
	char *p;
	int i;
	ssize_t sz;
	sz = write(fd, bytes, len);
	if (sz == -1) {
		fprintf(stderr, "write() failed: %d\n", errno);
		exit(10);
	}
	printf("%d]=> ", sz);
	for(p=bytes, i=0;i<len;i++,p++)
		printf("%02hhX ", *p);
	printf("\n");

}

void recv_and_print(int fd)
{
	ssize_t sz=0;
	int cnt = 0;
	char buf;
	printf("<== ");
	while (sz != -1) {
		sz = read(fd, &buf, 1);
		if (sz == -1) {
			if (errno == EAGAIN) {
				//printf("[%u]==\n", cnt);
				break;
			} else {
				printf("?? %d\n", errno);
			}
			return;
		} else {
			printf("%02hhX ", buf);
			cnt++;
		}
	}

	printf("[%u]==\n", cnt);
}

/*** main code ***/

void main_loop(int fd)
{
	char *rl = 0;
	char *bytes;
	int len;
	while (TRUE) {
		rl = readline("> ");
		if (rl == 0) {
			return;
		}
		add_history(rl);
		if (strncmp(rl, "baud ", 5) == 0) {
			long int l;
			char *p = rl+5;
			l = atoi(p);
			set_baud(fd, l);
			free(rl);
			rl = 0;
			continue;
		}
		bytes = parse_line(rl, &len);
		free(rl);
		rl = 0;
		send_data(fd, bytes, len);
		free(bytes);
		recv_and_print(fd);
	}
}

int main(int argc, char *argv[])
{
	int fd;
	struct termios term;
	if (argc != 2) {
		fprintf(stderr, "Usage: %s device\n", argv[0]);
		return 2;
	}
	fd = open(argv[1], O_RDWR | O_NOCTTY | O_NONBLOCK);
	if(fd < 0) {
		fprintf(stderr, "can't open %s\n", argv[1]);
		return 1;
	}
	printf("< opened %s\n", argv[1]);

	term.c_iflag=0x5;
	term.c_oflag=0;
	term.c_cflag=0x1cb2;
	term.c_lflag=0;
	term.c_line=0;
	memcpy(term.c_cc, "\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", NCCS);
	ioctl (fd, TCSETS, &term);
	printf("< configured port\n");
	main_loop(fd);
	close(fd);
	return 0;
}

