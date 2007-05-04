/* bcm2035-tool - bcm2035 tool for Motorola EZX phones
 *
 * (C) 2007 by Daniel Ribeiro <drwyrm@gmail.com>
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


#define HCIUARTSETPROTO _IOW('U', 200, int)

#define MAXWRITESIZE 16

struct cmd {
	char *cmd;
	int size;
	char *reply;
};

struct patch {
	__u16 address;
	__u8 flag;
	int size;
	char *data;
};

#define REP		"\x04\x0e\x04"

#define BAUD115200	"\x00\xf3"
#define BAUD460800	"\x22\xfd"
#define BAUD921600	"\x55\xff"

#define CMD_SET_BAUD	"\x18\xfc"
#define CMD_RESET	"\x03\x0c"
#define CMD_PATCH_FW	"\x2e\xfc"
#define CMD_SET_BDADDR	"\x01\xfc" 


