#ifndef __PROCESS_H__
#define __PROCESS_H__

#define START 0
#define STOP 1
#define RESTART 2
#define INT_SIZE sizeof(int)
#define WRITE_CODE(fd, cd) code = cd; write(fd, &code, INT_SIZE)

int code;

typedef struct {
	char * command;
	int pid;
	int restart_on_close;
	int output;
	int control;
} process;

process * process_new(char * command, int restart_on_close);
int start_process(process * proc);
void stop_process(process * proc);
void process_free(process * proc);

#endif /* __PROCESS_H__ */



