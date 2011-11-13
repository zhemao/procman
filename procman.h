#ifndef __PROCMAN_H__
#define __PROCMAN_H__

#include "process.h"
#include "notifications.h"

#include <stdio.h>
#include <sys/time.h>

#ifdef INOTIFY
	#include <sys/inotify.h>
	#define EVENT_SIZE (sizeof(struct inotify_event))
	#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))
	#define IN_MY_FLAGS IN_MODIFY|IN_CREATE|IN_DELETE|IN_MOVE
	#define TIMEOUT 2
#endif
	
process * proc;
FILE * out;

time_t last_modified;
fd_set master_set, working_set;

void print_usage(char * name);
char * join(char ** args, int len);
void output_callback(int fd);
void control_callback(int fd);
void handle_signal(int sig);
void check_for_change(char * filename);
time_t get_last_modified_time(char * filename);
void notify(notify_action action);

#endif /* __PROCMAN_H__ */


