/* bcm2035-tool
 *
 * (C) 2007 by Daniel Ribeiro <drwyrm@gmail.com>
 * 
 *  MAC address as argument extension by: Jan Herman <2hp@seznam.cz>
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
 */

/* Default MAC address is 01:02:03:04:05:06 */
#define BDADDR "\x06\x05\x04\x03\x02\x01"

//#define PATCH_FIRMWARE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include "bcm2035-tool.h"
#include "bluetooth.h"

#ifdef PATCH_FIRMWARE
#include "motorola_patches.h"

__u8 checksum(char *data, int size)
{
	__u8 ret = 0;
	int i;

	for(i=0; i<size; i++)
		ret-=data[i];
	return(ret);
}


char a_buffer[43];
char *generate_packet(__u8 kind, __u16 address, __u8 flag, __u8 size, char *data)
{
	char b_buffer[21];
	char t_buffer[2];
	__u8 y, i;
	__u8 x = 0;

	b_buffer[x++] = size;
	b_buffer[x++] = (__u8) (address >> 8);
	b_buffer[x++] = (__u8) address;
	if (kind) 
		b_buffer[x++] = flag;
	memcpy((char *)b_buffer + x, data, size);
	x+=size;
	b_buffer[x] = checksum(b_buffer, x);
	x++;

	for(i=0,y=0;i<x;i++)
	{
		sprintf(t_buffer, "%.2X", (__u8) b_buffer[i]);
		a_buffer[y++] = t_buffer[0];
		a_buffer[y++] = t_buffer[1];
	}
	a_buffer[y] = 0;
	return((char *)a_buffer);
}

int send_patch(int fd, struct patch p, __u8 kind)
{
	int n;
	char t[50];
	int ret = 0;
	// p.size == 0 has special treatment
	if (p.size == 0) {
		sprintf (t, ":%s", generate_packet(kind, p.address, p.flag,
							p.size, p.data));
		ret += write(fd, t, strlen(t));
		read (fd, t, 8);
		if (t[0] != '.')
			exit(-2);

	}
	else for (n=0;n<p.size;n+=MAXWRITESIZE) {
		sprintf (t, "%s%s", (kind?":":"@00"), generate_packet(kind, 
			p.address + n, p.flag, (p.size - n > MAXWRITESIZE ? 
			MAXWRITESIZE : p.size - n), p.data + n));
		ret += write(fd, t, strlen(t));
		read (fd, t, 8);
		if (!kind && strncmp(t, "@01", 3))
			exit (-3);
		if (kind && t[0] != '.')
			exit (-2);
		

	}
	return(ret);
}
#endif // PATCH_FIRMWARE

int send_cmd (int fd, char *cmd, char *arg, int argsize)
{
	char buf[16];
	int x = 0;
	buf[x++] = 0x01;
	memcpy(buf + x, cmd, 2);
	x+=2;
	buf[x++] = (__u8) argsize;
	if(argsize > 0)
	{
		memcpy(buf + x, arg, argsize);
		x+=argsize;
	}
	write(fd, buf, x);
	read(fd, buf, 8);
	if (strncmp(buf, REP, 3) != 0 || buf[3] != 1 || 
			strncmp(buf+4, cmd, 2) != 0 || buf[6] != 0) {
		printf("    unexpected reply\n");
		exit(-10);
	}

}
	
#ifdef PATCH_FIRMWARE
int patch_fw(int fd, struct patch p1[], struct patch p2[])
{
	int i;
	char buf[8];

	printf("Patching Firmware...\n");
	send_cmd(fd, CMD_PATCH_FW, NULL, 0);

	read(fd, buf, 8);

	for (i=0;p1[i].data!=NULL;i++)
		printf(":%d: Wrote %d bytes\n", i, send_patch(fd, p1[i], 1));

	for (i=0;p2[i].data!=NULL;i++)
		printf("@%d: Wrote %d bytes\n", i, send_patch(fd, p2[i], 0));

	write(fd, "Z", 1);
}
#endif

int set_baud(int fd, int baud)
{
	struct termios t;
	char buf[8];

	printf("Set baud: %d\n", baud);
	tcgetattr(fd, &t);
	switch (baud) {
		case 115200:
			send_cmd(fd, CMD_SET_BAUD, BAUD115200, 2);
        		cfsetospeed(&t, B115200);
			break;
		case 460800:
			send_cmd(fd, CMD_SET_BAUD, BAUD460800, 2);
			cfsetospeed(&t, B460800);
			break;
		case 921600:
			send_cmd(fd, CMD_SET_BAUD, BAUD921600, 2);
        		cfsetospeed(&t, B921600);
			break;
		default:
			printf ("invalid baud rate!\n");
			return(-1);
	}
	return tcsetattr(fd, TCSANOW, &t);

}

void mac2hex(const char *mac, u_char *dst)
{
	
     int i;
     long l;
     char *pp;
 
     while (isspace(*mac))
         mac++;
 
     /* expect 6 hex octets separated by ':' or space/NUL if last octet */
     for (i = 0; i < 6; i++) {
         l = strtol(mac, &pp, 16);
         if (pp == mac || l > 0xFF || l < 0)
             return;
         if (!(*pp == ':' || (i == 5 && (isspace(*pp) || *pp == '\0'))))
             return;
		 /* 5-i swaps hex chars */
		 dst[5-i] = (u_char) l;
         mac = pp + 1;
     }
 }

char *bdaddr[6];
 
int main(int argc, char **argv)
{
	int fd, i;
	struct termios term;
	char buf[1024];
	
	if (argc < 3) {
		printf("BCM2035 Tool:\n");
		printf("Usage: %s <device> <bdaddr>\n", argv[0]);
		exit(1);
	}
	
	/* MAC address is required */

	if (strlen(argv[2]) < 17) {
		printf("invalid argument %s!\n", argv[2]);
		exit(1);
	}
	
	/* Converts MAC address to swapped HEX */
	mac2hex(argv[2], &bdaddr);
	
	fd = open (argv[1], O_RDWR | O_NOCTTY );

	if(fd < 0) exit(-1);

	term.c_iflag=0x5;
	term.c_oflag=0;
	term.c_cflag=0x1cb2;
	term.c_lflag=0;
	term.c_line=0;
	memcpy(term.c_cc, "\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", NCCS);
	ioctl (fd, TCSETS, &term);

	send_cmd(fd, CMD_RESET, NULL, 0);
#ifdef PATCH_FIRMWARE
	set_baud(fd, 921600);
	patch_fw(fd, motorola_patches, motorola_firmware);
	sleep(5);
#endif
	set_baud(fd, 460800);
	
	send_cmd(fd, CMD_SET_BDADDR, bdaddr, 6);

        i = N_HCI;
        if (ioctl(fd, TIOCSETD, &i) < 0) {
                perror("Can't set line discipline");
                exit(1);
        }
        if (ioctl(fd, HCIUARTSETPROTO, 0) < 0) {
                perror("Can't set device");
                exit(1);
        }
        if (fork()) 
		return 0;
        while (1) sleep(999999999);
        return 0;
}
