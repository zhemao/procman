#ifndef __PROCMAN_H__
#define __PROCMAN_H__

#include "process.h"
#include <libnotify/notify.h>
#include <libnotify/notification.h>

#define UNINIT_AND_EXIT notify_uninit(); exit(0)
	
process * proc;
FILE * out;
NotifyNotification * start_note;
NotifyNotification * restart_note;
NotifyNotification * modify_note;
NotifyNotification * stop_note;
time_t last_modified;
fd_set master_set, working_set;

void print_usage(char * name);
char * join(char ** args, int len);
void output_callback(int fd);
void control_callback(int fd);
void handle_signal(int sig);
void check_for_change(char * filename);
time_t get_last_modified_time(char * filename);

#endif /* __PROCMAN_H__ */


