#ifndef _MACH_H_
#define _MACH_H_

int determine_model(void);

struct model_t {
	char *name;
	char *shortname;

	/* hook functions */
	void (*scan_partitions_hook)(void);
};

extern struct model_t *model;

#endif
