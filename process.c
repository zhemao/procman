#include "process.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>

process * process_new(char * command, int argc, char **argv, 
		int restart_on_close){
	int len = strlen(command);
	int i;
	
	process * proc = (process*)malloc(sizeof(process));
	proc->command = (char*)malloc(sizeof(char)*(len+1));
	strcpy(proc->command, command);
	
	proc->numargs = argc;
	proc->args = (char**)malloc(sizeof(char*) * (argc+1));
	
	for(i=0; i<argc; i++){
		len = strlen(argv[i]);
		proc->args[i] = malloc(sizeof(char*) * (len+1));
		strcpy(proc->args[i], argv[i]);
	}
	
	proc->args[argc] = NULL;
	proc->restart_on_close = restart_on_close;
	return proc;
}

int start_process(process * proc){
	int output_pipe[2];
	
	/* set up our pipes */
	pipe(output_pipe);
	
	proc->output = output_pipe[0];
	
	/* fork, fork, fork ! */
	proc->pid = fork();
	
	/* if there was an error, return -1 */
	if(proc->pid == -1){
		printf("fork failed for some reason\n");
		return -1;
	}
	if(proc->pid == 0){
		/* open a file descriptor to /dev/null */
		int devnull = open("/dev/null", O_RDONLY);
		int retcode;
		
		/* close the input pipes */
		close(output_pipe[0]);
		
		/* redirect stdin to /dev/null */
		dup2(devnull, 0);
		
		/* redirect stdout and stderr to the pipe */
		dup2(output_pipe[1], 1);
		dup2(output_pipe[1], 2);
		
		retcode = execvp(proc->command, proc->args);
		exit(retcode);
	}
	
	/* close the output ends */
	close(output_pipe[1]);
	
	fcntl(proc->output, F_SETFL, O_NONBLOCK);
	
	return 0;
}

void stop_process(process * proc){
	close(proc->output);
	kill(proc->pid, SIGINT);
	kill(proc->pid, SIGTERM);
	kill(proc->pid, SIGKILL);
}

void process_free(process * proc){
	int i;
	for(i=0; i<proc->numargs; i++)
		free(proc->args[i]);
	free(proc->args);
	free(proc->command);
	free(proc);
}
