#include "procman.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#ifdef INOTIFY
	#include <sys/inotify.h>
#endif

static int fdmax;

void print_usage(char * name){
	printf("Usage: %s [options] command [command args ...]\n", name);
	printf("Options:\n");
	printf("-r Restart - set to 0 or 1 to disable or enable restart on close\n");
	printf("\tdefaults to 1\n");
	printf("-w Watch - set to 0 or 1 to disable or enable watching of the executable\n");
	printf("\tdefaults to 1\n");
	printf("-o Output - output to the given file instead of stdout\n");
	exit(1);
}

char * join(char ** args, int len){
	int i, size = 0;
	char * buf;
	for(i=0; i<len; i++){
		size += strlen(args[i]);
	}
	buf = (char*)malloc(sizeof(char)*(size+1));
	strcpy(buf, args[0]);
	for(i=1; i<len; i++){
		strcat(buf, args[i]);
	}
	return buf;
}

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

void output_callback(int fd){
	char buf[1024];
	
	memset(buf, 0, 1024);
	
	if(read(fd, buf, 1023) > 0){
		fprintf(out, "%s", buf);
	} else {
		if(proc->restart_on_close){
			notify(RESTART);
			start_process(proc);
			FD_ZERO(&master_set);
			FD_SET(proc->output, &master_set);
		} else {
			notify(STOP);
			UNINIT_AND_EXIT;
		}
		
	}
}

void handle_signal(int sig){
	stop_process(proc);
	UNINIT_AND_EXIT;
}

time_t get_last_modified_time(char * filename){
	struct stat st;
	if(stat(filename, &st)==0){
		return st.st_mtime;
	}
	return 0;
}

#ifdef INOTIFY
void inot_callback(int fd, char * filename){
	struct inotify_event event;
	if(read(fd, &event, sizeof(event)) > 0){
		notify(MODIFY);
		stop_process(proc);
		start_process(proc);
		FD_ZERO(&master_set);
		FD_SET(proc->output, &master_set);
		FD_SET(fd, &master_set);
		fdmax = (fd > proc->output) ? fd : proc->output;
	}
}
#endif

void check_for_change(char * filename){
	if(filename == NULL) return;
	time_t mtime = get_last_modified_time(filename);
	if(mtime > last_modified){
		notify(MODIFY);
		stop_process(proc);
		start_process(proc);
		FD_ZERO(&master_set);
		FD_SET(proc->output, &master_set);
		fdmax = proc->output;
		last_modified = mtime;
	}
}

int main(int argc, char *argv[]){
	int restart_on_close = 1, watch = 1, nohup = 0, opt;
	char * command;
	int retcode;
	struct timeval timeout;
	out = stdout;
#ifdef INOTIFY
	int inotfd;
#endif
	
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
	
	while((opt = getopt(argc, argv, "r:w:o:n")) != -1){
		switch(opt){
		case 'r': restart_on_close = atoi(optarg);
			break;
		case 'w': watch = atoi(optarg);
			break;
		case 'o': out = fopen(optarg, "w");
			break;
		case 'n': nohup = 1;
			break;
		default: print_usage(argv[0]);
		}
	}
	
	if(optind >= argc)
		print_usage(argv[0]);
		
	command = argv[optind];
	if(watch){
	#ifdef INOTIFY
		inotfd = inotify_init1(IN_NONBLOCK);
		inotify_add_watch(inotfd, command, IN_ATTRIB|IN_MODIFY|IN_CLOSE_WRITE);
	#else
		last_modified = get_last_modified_time(command);
	#endif
	}
	proc = process_new(command, argc-optind, argv+optind, restart_on_close);
	
	retcode = start_process(proc);
	notify(START);
	
	timeout.tv_sec  = 2;
	timeout.tv_usec = 0;
	
	if(retcode != -1){
	
		signal(SIGINT, handle_signal);
		signal(SIGTERM, handle_signal);
		if(nohup)
			signal(SIGHUP, SIG_IGN);
		else signal(SIGHUP, handle_signal);

		fdmax = proc->output;
	
		FD_ZERO(&master_set);
		FD_SET(proc->output, &master_set);
		#ifdef INOTIFY
		if(watch){
			FD_SET(inotfd, &master_set);
			fdmax = (inotfd > proc->output) ? inotfd : proc->output;
		} 
		#endif

		while(1){
			memcpy(&working_set, &master_set, sizeof(master_set));
			retcode = select(fdmax+1, &working_set, 
					NULL, NULL, &timeout);
			
			if(retcode < 0){
				printf("select() failed ... exiting \n");
				exit(-1);
			}
			
			if(retcode > 0) {
			
				if(FD_ISSET(proc->output, &working_set)){
					output_callback(proc->output);
				}

				#ifdef INOTIFY
				if(watch && FD_ISSET(inotfd, &working_set)){
					inot_callback(inotfd, proc->command);	
				}
				#endif
			}
			#ifndef INOTIFY
			if(watch) check_for_change(proc->command);
			#endif
		}
				
		stop_process(proc);
	}
	
	notify_uninit();
	
	return 0;
}
