#include "procman.h"
#include "process.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>

process * proc;
FILE * out;
char * towatch;

void print_usage(char * name){
	printf("Usage: %s [options] command ...\n", name);
	printf("Options:\n");
	printf("-n No Restart - do not restart when process exits\n");
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
		exit(0);
	}
}

void control_callback(int fd){
	int code;
	
	if(read(fd, &code, sizeof(int)) > 0){
		if(code == STOP){
			printf("process stopped ... exiting\n");
			exit(0);
		}
		if(code == START){
			printf("process started\n");
		} 
		else if(code == RESTART){
			printf("process stopped ... restarting\n");
		}
	}
}

void handle_signal(int sig){
	stop_process(proc);
	exit(0);
}

int main(int argc, char *argv[]){
	int restart_on_close = 1, opt, i, fd_max;
	char * command;
	int retcode;
	fd_set master_set, working_set;
	struct timeval timeout;
	out = stdout;
	
	while((opt = getopt(argc, argv, "no:")) != -1){
		switch(opt){
		case 'n': restart_on_close = 0;
			break;
		case 'o': out = fopen(optarg, "w");
			break;
		default: print_usage(argv[0]);
		}
	}
	
	if(optind >= argc)
		print_usage(argv[0]);
		
	command = join(argv+optind, argc-optind);
	proc = process_new(command, restart_on_close);
	free(command);
	
	retcode = start_process(proc);
	if(proc->output > proc->control)
		fd_max = proc->output+1;
	else fd_max = proc->control+1;
	
	timeout.tv_sec  = 10;
	timeout.tv_usec = 0;
	
	if(retcode != -1){
	
		signal(SIGINT, handle_signal);
		signal(SIGTERM, handle_signal);
	
		FD_ZERO(&master_set);
		FD_SET(proc->output, &master_set);
		FD_SET(proc->control, &master_set);
		
		while(1){
			memcpy(&working_set, &master_set, sizeof(master_set));
			retcode = select(fd_max, &working_set, NULL, NULL, &timeout);
			
			if(retcode < 0){
				printf("select() failed ... exiting \n");
				exit(-1);
			}
			
			if(retcode > 0) {
			
				if(FD_ISSET(proc->output, &working_set)){
					output_callback(proc->output);
				}
				
				if(FD_ISSET(proc->control, &working_set)){
					control_callback(proc->control);
				}
			}
		}
		
		stop_process(proc);
	}
	
	return 0;
}