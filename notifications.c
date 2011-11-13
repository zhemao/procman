#include "notifications.h"

#ifdef LIBNOTIFY
void notify(notify_action action){
	NotifyNotification * note;
	switch(action){
	case START: note = start_note;
		break;
	case RESTART: note = restart_note;
		break;
	case STOP: note = stop_note;
		break;
	case MODIFY: note = modify_note;
	}
	notify_notification_show(note, NULL);
}
#else
void notify(notify_action action){
	char * message;
	switch(action){
	case START: message = start_message;
		break;
	case RESTART: message = restart_message;
		break;
	case STOP: message = stop_message;
		break;
	case MODIFY: message = modify_message;
	}
	printf("%s\n", message);
}
#endif

void init_notifications(){
	#ifdef LIBNOTIFY
	notify_init("procman");
	start_note = notify_notification_new("Procman Notification", 
					"Your process has been started", NULL);
	stop_note = notify_notification_new("Procman Notification",
					"Your process has stopped ... procman exiting now", 
					NULL);
	restart_note = notify_notification_new("Procman Notification",
					"Your process has stopped ... restarting", NULL);
	modify_note = notify_notification_new("Procman Notification",
					"Procman has detected a change in your program. Restarting the process.", 
					NULL);
	#else
	start_message = "Your process has been started";
	stop_message = "Your process has stopped ... procman exiting now";
	restart_message = "Your process has stopped ... restarting";
	modify_message = "Procman has detected a change in your program. Restarting the process.";
	#endif
}
