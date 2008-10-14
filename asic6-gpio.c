/*
 * gpio-val.c: Simple program to read/write GPIO levels.
 *
 * Alex Osborne <ato@meshy.org> 2008
 *
 * Based devmem2.c by
 *
 *  Copyright (C) 2000, Jan-Derk Bakker (jdb@lartmaker.nl)
 *
 *
 * This software has been developed for the LART computing board
 * (http://www.lart.tudelft.nl/). The development has been sponsored by
 * the Mobile MultiMedia Communications (http://www.mmc.tudelft.nl/)
 * and Ubiquitous Communications (http://www.ubicom.tudelft.nl/)
 * projects.
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/mman.h>
  
#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", \
  __LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)
 
#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

typedef unsigned long u32;


int regoff(int gpio) {
	if (gpio < 32) return 0;
	if (gpio < 64) return 4;
	if (gpio < 96) return 8;
	return 0x100;
}

int gpio_get(void *map_base, int gpio) {
	volatile u32 *reg = (u32*)((u32)map_base + 0x48 + regoff(gpio));
	return (*reg >> (gpio&31)) & 1;
}

void gpio_set(void *map_base, int gpio, int val) {
	volatile u32 *reg = (u32*)((u32)map_base + 0x48 + regoff(gpio));
	if (val)
		*reg |= 1 << (gpio & 31);
	else
		*reg &= ~(1 << (gpio & 31));
}


int main(int argc, char **argv) {
    int fd;
    void *map_base, *virt_addr; 
	unsigned long read_result, writeval;
	off_t target;
	int access_type = 'w';
	
	if(argc < 2) {
		fprintf(stderr, "\nUsage:\t%s gpio [ value ]\n"
			"\tgpios    : gpio number to act upon\n"
			"\tvalue    : valie to set\n\n",
			argv[0]);
		exit(1);
	}


    if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) FATAL;
    fflush(stdout);
    
    /* Map one page */
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x08000000);
    if(map_base == (void *) -1) FATAL;

	int gpio = atoi(argv[1]);

	if (argc > 2) {
		gpio_set(map_base, gpio, atoi(argv[2]));
	}

	printf("gpio %d %d\n", gpio, gpio_get(map_base, gpio));
    
    virt_addr = map_base + (target & MAP_MASK);
    read_result = *((unsigned long *) virt_addr);
    if(munmap(map_base, MAP_SIZE) == -1) FATAL;
    close(fd);
    return 0;
}

