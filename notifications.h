#ifdef LIBNOTIFY
	#include <libnotify/notify.h>
	#include <libnotify/notification.h>
#endif

#ifdef LIBNOTIFY
	#define UNINIT_AND_EXIT notify_uninit(); exit(0)
#else
	#define UNINIT_AND_EXIT exit(0)
#endif

typedef enum {
	START,
	RESTART,
	MODIFY,
	STOP
} notify_action;

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

void notify(notify_action action);
void init_notifications();
