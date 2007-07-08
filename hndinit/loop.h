#ifndef _LOOP_H_
#define _LOOP_H_

char *query_loop(const char *device);
int del_loop(const char *device);
int set_loop(char **device, const char *file, int offset);

#endif
