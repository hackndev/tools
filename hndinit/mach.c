/* mach.c - machine-speficic configuration for hndinit
 *
 * Copyright (C) 2007 by Alex Osborne <bobofdoom@gmail.com>
 *
 * Licensed under the GPL v2 or later, see the file LICENSE.
 */

#include "hndinit.h"
#include <sys/types.h>                                                                                                                                                         
#include <sys/stat.h>
#include <unistd.h>  

static void palmld_scan_partition(char *rawdev)
{
	char *dev = NULL;
	int ret;
	struct stat statbuf;

	if (stat(rawdev, &statbuf) == -1)
		return;
	
	ret = set_loop(&dev, rawdev, 91814912);
	if (ret < 0) {
		eerror("   hda3: set_loop: %s", strerror(ret));
		return;
	}

	if (scan_partition(dev + 5) != 0) {
		del_loop(dev);
	}

	free(dev);
}

static void palmld_scan_partitions(void)
{
	einfo("Scanning LifeDrive data partition...");
	palmld_scan_partition("/dev/hda");
	palmld_scan_partition("/dev/mmcblk0"); /* for testing HDD images in qemu */
}

static struct model_t models[] = {
	{
		.name = "Generic",
		.shortname = "generic",
	},
	{
		.name = "Palm LifeDrive",
		.shortname = "palmld",
		.scan_partitions_hook = palmld_scan_partitions,
	},
	{
		.name = NULL, /* sentinal */
	},
};

struct model_t *model = &models[0];

static void set_model(char *name)
{
	struct model_t *m = models;

	while (m->name) {
		if (!strcasecmp(name, m->name)) {
			model = m;
			return;
		}
		m++;
	}
	
	/* not found, default to generic model, but with our name */
	model = &models[0];
	model->name = strdup(name);
}

/**
 * Determine the system's hardware model by looking in /proc/cpuinfo.
 */
int determine_model(void)
{
	FILE *f;
	char line[256];
	char *model_s = NULL;
	char *newline = NULL;

	ebegin("Checking model... ");

	f = fopen("/proc/cpuinfo", "r");

	if (!f) {
		eend(-1);
		eerror("/proc/cpuinfo: %s", strerror(errno));
		return -1;
	}

	while (fgets(line, 255, f)) {
		if (!strncmp(line, "Hardware", 8)) {
			model_s = index(line, ':') + 2;

			/* strip trailing \n */
			newline = index(line, '\n');
			if (newline) *newline = '\0';

			set_model(model_s);
			printf("%s (%s)", model->name, model->shortname);
			break;
		}
	}

	fclose(f);

	if (!model_s) {
		eend(-1);
		eerror("Hardware field not found in cpuinfo");
		return -1;
	}

	return eend(0);
}


