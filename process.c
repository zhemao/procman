#include "process.h"
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>


process * process_new(char * command, int restart_on_close){
	int len = strlen(command);
	process * proc = (process*)malloc(sizeof(process));
	proc->command = (char*)malloc(sizeof(char)*(len+1));
	strcpy(proc->command, command);
	proc->restart_on_close = restart_on_close;
	return proc;
}

int start_process(process * proc){
	int output_pipe[2];
	int control_pipe[2];
	
	/* set up our pipes */
	pipe(output_pipe);
	pipe(control_pipe);
	
	proc->output = output_pipe[0];
	proc->control = control_pipe[0];
	
	/* fork, fork, fork ! */
	proc->pid = fork();
	
	/* if there was an error, return -1 */
	if(proc->pid == -1){
		return -1;
	}
	if(proc->pid == 0){
		/* open a file descriptor to /dev/null */
		int devnull = open("/dev/null", O_RDONLY);
		int retcode;
		
		/* close the input pipes */
		close(output_pipe[0]);
		close(control_pipe[0]);
		
		/* redirect stdin to /dev/null */
		dup2(devnull, 0);
		
		/* redirect stdout and stderr to the pipe */
		dup2(output_pipe[1], 1);
		dup2(output_pipe[1], 2);
		
		if(proc->restart_on_close){
			while(1){
				WRITE_CODE(control_pipe[1], START);
				retcode = system(proc->command);
				WRITE_CODE(control_pipe[1], RESTART);
			}
			
			/* Ahh, what the hell happened? We shouldn't be here! */
			exit(-1);
		}
		
		WRITE_CODE(control_pipe[1], START);
		retcode = system(proc->command);
		WRITE_CODE(control_pipe[1], STOP);
		exit(retcode);
	}
	
	/* close the output ends */
	close(output_pipe[1]);
	close(control_pipe[1]);
	
	fcntl(proc->output, F_SETFL, O_NONBLOCK);
	fcntl(proc->control, F_SETFL, O_NONBLOCK);
	
	return 0;
}

void stop_process(process * proc){
	kill(proc->pid, SIGINT);
	kill(proc->pid, SIGTERM);
	kill(proc->pid, SIGKILL);
}

void process_free(process * proc){
	free(proc->command);
	free(proc);
}
