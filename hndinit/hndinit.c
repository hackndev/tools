/* hndinit.c - init binary for tiny initramfs
 *
 * Copyright (C) 2007 by Alex Osborne <bobofdoom@gmail.com>
 *
 * Licensed under the GPL v2 or later, see the file LICENSE.
 */

#include "hndinit.h"

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/reboot.h>
#include <fcntl.h>
#include <unistd.h>

struct rootfs_list_t {
	char *dev;
	char *filename;
	struct rootfs_list_t *next;
} *rootfs_list = NULL;

#define KEY_UP   65
#define KEY_DOWN 66

/**
 * Do a mount but display pretty status messages.
 */
static int emount(const char *source, const char *target,                                                                                                                              		const char *filesystemtype, unsigned long mountflags,                                                                                                                
		const void *data)
{
	int ret, err;
	ebegin("Mounting %s on %s...", source, target);
	ret = mount(source, target, filesystemtype, mountflags, data);
	err = errno;
	eend(ret);

	if (ret) {
		eerror("mount: %s", strerror(errno));
	}
	return ret;
}

static int dbg_cat(char *fn)
{
	FILE *f;
	char line[256];
	f = fopen(fn, "r");
	if (!f) {
		eerror("%s: %s", fn, strerror(errno));
		return -1;
	}

	while (fgets(line, 255, f))
		printf("%s", line);
	fclose(f);
	return 0;
}

/**
 * Compress the whitespace in the given string.
 */
static char *compress_ws(char *s)
{
	char *tmp = strdup(s);
	char *t = tmp;
	int last_sp = 0;
	char *string = s;

	/* skip leading spaces */
	while (isspace(*t)) t++;

	while (*t) {
		if (!isspace(*t) || !last_sp) {
			*s++ = *t;
		}
		last_sp = isspace(*t);
		t++;
	}
	*s = 0;
	free(tmp);
	return string;
}

/**
 * Sets the kernel log level enabling us to suppress kernel messages.
 */
int set_kernel_loglevel(int level)
{
	FILE *f = fopen("/proc/sys/kernel/printk", "w");
	if (!f) return -1;
	fprintf(f, "%d\n", level);
	fclose(f);
	return 0;
}

int try_mount(const char *dev, const char *mountpoint)
{
	char line[256];
	FILE *f=fopen("/proc/filesystems", "r");
	int ret = -1;

	if (!f) {
		eerror("fopen /proc/filesystems: %s", strerror(errno));
		return -1;
	}

	/* suppress filesystem error messages */
	set_kernel_loglevel(3);

	while (fgets(line, 255, f)) {
		compress_ws(line);
		
		if (!strncmp(line, "nodev", 5)) continue;
		*index(line, '\n') = 0;

		/* force trying vfat before msdos */
		if (!strcmp(line, "msdos")) {
			if (mount(dev, mountpoint, "vfat", 0, NULL) == 0) {
				ret = 0;
				break;
			}
		}

		if (mount(dev, mountpoint, line, 0, NULL) == 0) {
			ret = 0;
			break;
		}
	}
	
	set_kernel_loglevel(7);

	fclose(f);
	return ret;
}

/**
 * Return non-zero if s ends with ext.
 */
int endswith(char *s, char *ext)
{
	int i = strlen(s) - strlen(ext);
	if (i < 0) return 0;
	return !strcmp(s + i, ext);
}

/**
 * Add a new entry to the rootfs list.
 */
void add_rootfs(char *devname, char *filename)
{
	struct rootfs_list_t *rootfs = malloc(sizeof(struct rootfs_list_t));
	struct rootfs_list_t *item = rootfs_list;

	if (item == NULL) {
		item = rootfs_list = rootfs;
	} else {
		while (item->next)
			item = item->next;
		item->next = rootfs;
	}

	rootfs->filename = strdup(filename);
	rootfs->dev = strdup(devname);
	rootfs->next = NULL;
}

/**
 * Mount and scan a partition looking for rootfs images.
 */
int scan_partition(char *devname)
{
	char mountpoint[256];
	char devpath[256];
	DIR *d;
	struct dirent *entry;

	snprintf(mountpoint, sizeof(mountpoint)-1, "/mnt/%s", devname);
	snprintf(devpath, sizeof(devpath)-1, "/dev/%s", devname);

	if (mkdir(mountpoint, 0755) == -1 && errno != EEXIST) {
		eerror("mkdir %s: %s", mountpoint, strerror(errno));
		return -1;
	}

	if (try_mount(devpath, mountpoint) == -1) {
		ewarn("    %s is unmountable", devname);
		return -1;
	}

	einfo("    %s", devname);

	d = opendir(mountpoint);
	if (d == NULL) {
		eerror("opendir %s: %s", mountpoint, strerror(errno));
		return -1;
	}

	while ((entry = readdir(d)) != NULL) {
		if (endswith(entry->d_name, ".ext") 
				|| endswith(entry->d_name, ".ext2")
				|| strstr(entry->d_name, ".rootfs")) {
			einfo(BLUEF BOLDON "    |-->" RESET " %s\n", entry->d_name);
			add_rootfs(devname, entry->d_name);
		}
	}

	closedir(d);

	return 0;
}

/**
 * Ensure the device node /dev/devname exists.
 */
int mkpartnode(char *devname, int major, int minor)
{
	char path[256];

	snprintf(path, sizeof(path)-1, "/dev/%s", devname);

	if (mknod(path, S_IFBLK, makedev(major, minor)) == -1 && errno != EEXIST) {
		eerror("mknod %s: %s", path, strerror(errno));
		return -1;
	}
	return 0;
}

/**
 * Attempt to mount each partition in /proc/partitions in turn
 * scanning for rootfs.
 */
static int scan_partitions(void)
{
	FILE *f;
	char line[256];
	char *s;
	int major, minor;
	long blocks;
	char devname[256];

	einfo("Scanning partitions...");

	f = fopen("/proc/partitions", "r");
	if (!f) {
		eerror("/proc/partitions: %s", strerror(errno));
		return -1;
	}
	
	/* skip first two lines */
	fgets(line, 255, f);
	fgets(line, 255, f);

	while (fgets(line, 255, f)) {
		s = compress_ws(line);
		
		major = atoi(s);  while (!isspace(*s++));
		minor = atoi(s);  while (!isspace(*s++));
		blocks = atol(s); while (!isspace(*s++));
		strcpy(devname, s);
		devname[strlen(devname)-1] = 0; /* kill \n */

		mkpartnode(devname, major, minor); /* create device node */
		scan_partition(devname);

	}
	fclose(f);

	if (model->scan_partitions_hook)
		model->scan_partitions_hook();

	return 0;
}

void umount_all(void)
{
	DIR *d;
	struct dirent *entry;
	char path[256];
	
	ebegin("Unmounting unused filesystems...");

	d = opendir("/mnt");
	if (d == NULL) {
		eend(-1);
		eerror("opendir %s: %s", "/mnt", strerror(errno));
		return;
	}

	while ((entry = readdir(d)) != NULL) {
		snprintf(path, sizeof(path)-1, "/mnt/%s", entry->d_name);
		umount(path);
	}

	closedir(d);
	eend(0);
}

int boot_rootfs(char *dev, char *filename)
{
	char path[512];
	char devpath[512];
	char *loopdev = NULL;
	int ret;

	printf("Booting\n");

	if (filename) {
		snprintf(path, sizeof(path)-1, "/mnt/%s/%s", dev, filename);
		ebegin("Configuring loopback device...");
		ret = set_loop(&loopdev, path, 0);
		if (ret != 0 && ret != 1) {
			eend(-1);
			eerror("%s: set_loop: %s", path, strerror(ret));
			return -1;
		}
		strncpy(devpath, loopdev, sizeof(devpath)-1);
		eend(0);
	} else {
		snprintf(devpath, sizeof(devpath)-1, "/dev/%s", dev);
	}

	ebegin("Creating /newroot...");
	if (mkdir("/newroot", 0755) == -1 && errno != EEXIST) {
		eend(-1);
		eerror("mkdir %s: %s", "/newroot", strerror(errno));
		return -1;
	}
	eend(0);

	ebegin("Mounting /newroot...");
	if (try_mount(devpath, "/newroot") != 0) {
		eend(-1);
		eerror("mount %s %s: %s", devpath, "/newroot", strerror(errno));
		return -1;
	}
	eend(0);

	umount_all();

	ebegin("Chrooting to /newroot...");
	if (chroot("/newroot") != 0) {
		eend(-1);
		eerror("chroot: %s\n", strerror(errno));
		return -1;
	}
	eend(0);

	chdir("/");
	
	einfo("Giving control to init");
	execl("/init", "", (char*)NULL);
	execl("/sbin/init", "", (char*)NULL);
	execl("/linuxrc", "", (char*)NULL);
	execl("/bin/sh", "", (char*)NULL);
	eerror("Failed to execute init, linuxrc or sh: %s", strerror(errno));
	return -1;
}

void rootfs_menu(void)
{
	unsigned int selected = 0, len = 0;
	int i, c;
	struct rootfs_list_t *rootfs, *selected_rootfs = NULL;

	while (1) {
		clear_screen();
		printf("\n");
		printf(BOLDON GREENF); draw_hr(); printf(RESET);
		printfc(BOLDON BLUEF"Select a Rootfs\n"RESET);
		printf(BOLDON GREENF); draw_hr(); printf(RESET);
		printf("\n");

		i = 0;
		for (rootfs = rootfs_list; rootfs; rootfs = rootfs->next) {
			if (i == selected) {
				printfc(BOLDON YELLOWF "%s/%s\n" RESET, rootfs->dev, rootfs->filename);
				selected_rootfs = rootfs;
			} else {
				printfc( "%s/%s\n", rootfs->dev, rootfs->filename);
			}
			i++;
		}

		len = i;

		c = getch();

		if (c == 27 && getch() == 91) { /* arrow keys */
			c = getch();

			if (c == KEY_UP) {
				if (selected == 0)
					selected = len;
				selected--;
			} else if (c == KEY_DOWN) {
				selected++;
			}
		} else if (c == 10 && selected_rootfs) { /* enter */
			boot_rootfs(selected_rootfs->dev, selected_rootfs->filename);
			printf("\n");
			einfo("Press any key to continue.");
			getch();
		}

		selected %= len;
	}

}

int main(int argc, const char **argv)
{
	refresh_winsize();
	clear_screen();

	printf("\n");
	printf(BOLDON GREENF); draw_hr(); printf(RESET);
	printfc(BOLDON BLUEF "Welcome to Linux4Palm\n" RESET);
	printfc(YELLOWF "www.hackndev.com\n" RESET);
	printf(BOLDON GREENF); draw_hr(); printf(RESET);
	printf("\n");

	emount("proc", "/proc", "proc", 0, NULL);
	emount("sysfs", "/sys", "sysfs", 0, NULL);

	determine_model();
	scan_partitions();

	add_rootfs("mmcblk0p1", "Fake-Entry.rootfs.ext2");
	add_rootfs("mmcblk0p1", "Fake-Entry2.rootfs.ext2");
	add_rootfs("hda4", "Fake-Entry3.rootfs.ext2");

	if (!rootfs_list) {
		eerror("No rootfs was found!");
		ewarn("");
		ewarn("Make sure you placed your rootfs in the");
		ewarn("root directory of an attached disk.");
		ewarn("");
		ewarn("Also ensure the name contains .rootfs");
		ewarn("or ends with .ext or .ext2");
		ewarn("");
		ewarn("Press any key to reboot.");
		getchar();
		reboot(LINUX_REBOOT_CMD_CAD_OFF);
		reboot(LINUX_REBOOT_CMD_RESTART);
		exit(-1);
	}

	rootfs_menu();

	return 0;
}
