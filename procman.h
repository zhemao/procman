#ifndef __PROCMAN_H__
#define __PROCMAN_H__

#include "process.h"
#include <stdio.h>
#include <sys/time.h>

#ifdef LIBNOTIFY
	#include <libnotify/notify.h>
	#include <libnotify/notification.h>
#endif

#ifdef LIBNOTIFY
	#define UNINIT_AND_EXIT notify_uninit(); exit(0)
#else
	#define UNINIT_AND_EXIT exit(0)
#endif

#ifdef INOTIFY
	#include <sys/inotify.h>
	#define EVENT_SIZE (sizeof(struct inotify_event))
	#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))
	#define IN_MY_FLAGS IN_MODIFY|IN_CREATE|IN_DELETE|IN_MOVE
#endif

typedef enum {
	START,
	RESTART,
	MODIFY,
	STOP
} notify_action;
	
process * proc;
FILE * out;

#ifdef LIBNOTIFY
	NotifyNotification * start_note;
	NotifyNotification * restart_note;
	NotifyNotification * modify_note;
	NotifyNotification * stop_note;
#else
	char * start_message;
	char * restart_message;
	char * modify_message;
	char * stop_message;
#endif



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


