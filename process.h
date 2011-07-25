#ifndef __PROCESS_H__
#define __PROCESS_H__

typedef enum {
	START,
	RESTART,
	MODIFY,
	STOP
} notify_action;

#define INT_SIZE sizeof(int)
#define WRITE_CODE(fd, cd) code = cd; write(fd, &code, INT_SIZE)

int code;

typedef struct {
	char * command;
	char ** args;
	int numargs;
	int pid;
	int restart_on_close;
	int output;
} process;

process * process_new(char * command, int argc, char ** argv,  int restart_on_close);
int start_process(process * proc);
void stop_process(process * proc);
void process_free(process * proc);

#endif /* __PROCESS_H__ */



