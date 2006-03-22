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

#define LOG(x) { fprintf(stderr, x); fflush(stderr); }

int main(int argc, char *argv[])
{
	int fd;
	unsigned long len;
	void *map;
	void *va;

	if(argc != 2) {
		fprintf(stderr, "Usage: dump-rom <size in Mb> > outfile\n");
		exit(1);
	}
	len = strtoul(argv[1], 0, 0) * 1024 * 1024;

	fprintf(stderr, "Dumping from 0 to %lu\n", len);
	fflush(stderr);
	if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
		LOG("Error opening /dev/mem\n");
		exit(2);
	}
	LOG("/dev/mem opened\n");

	map = mmap(0, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(map == MAP_FAILED) {
		LOG("mmap failed\n");
		exit(2);
	}
	fprintf(stderr, "Mapped to %p\n", map);
	fflush(stderr);

	/* dumping in native byte order */
	for(unsigned long i = 0; i<len; i++) {
		int val;
		va = map + i;
		val = *((unsigned char *) va);
		putchar(val);
		if(i % (1024*1024) == 0) LOG(".");
	}
	LOG("\n");
	
	if(munmap(map, len) == -1) {
		LOG("munmap failed\n");
		exit(2);
	}
	close(fd);

	return 0;
}

