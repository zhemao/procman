#include "procman.h"

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

static int fdmax;
#ifdef INOTIFY
static int inotfd;
#endif

void print_usage(char * name){
	printf("Usage: %s [options] command [command args ...]\n", name);
	printf("Options:\n");
	printf("-r Restart - set to 0 or 1 to disable or enable restart on close\n");
	printf("-w Watch - set to 0 or 1 to disable or enable watching of the executable\n");
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
	close(inotfd);
	stop_process(proc);
	UNINIT_AND_EXIT;
}

#ifdef INOTIFY
void inot_callback(int fd){
	int i = 0;
	char buffer[EVENT_BUF_LEN];
	time_t mtime = time(0);
	int length;

	if(mtime - last_modified < TIMEOUT) return;

	last_modified = mtime;
	
	length = read(fd, buffer, EVENT_BUF_LEN);

	while( i < length ){
		struct inotify_event *event = (struct inotify_event *) &buffer[i];
		if( event->name[0] != '.' ){
			notify(MODIFY);
			stop_process(proc);
			start_process(proc);
			FD_ZERO(&master_set);
			FD_SET(proc->output, &master_set);
			FD_SET(fd, &master_set);
			fdmax = (fd > proc->output) ? fd : proc->output;
			return;
		}
		i += EVENT_SIZE + event->len;
	}
}
#endif

int main(int argc, char *argv[]){
	int restart_on_close = 1, watch = 1, nohup = 0, opt;
	char * command;
	int retcode;
	out = stdout;
	char * cwd;
	
	init_notifications();
	
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
		cwd = getcwd(NULL, 0);
		inotify_add_watch(inotfd, cwd, IN_MY_FLAGS);
		free(cwd);
		last_modified = time(0);
	#endif
	}
	proc = process_new(command, argc-optind, argv+optind, restart_on_close);
	
	retcode = start_process(proc);
	notify(START);
	
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
					NULL, NULL, NULL);
			
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
					inot_callback(inotfd);	
				}
				#endif
			}
		}
				
		stop_process(proc);
		close(inotfd);
	}
	
	#ifdef LIBNOTIFY
	notify_uninit();
	#endif
	
	return 0;
}
