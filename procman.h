#ifndef __PROCMAN_H__
#define __PROCMAN_H__

void print_usage(char * name);
char * join(char ** args, int len);
void output_callback(int fd);
void control_callback(int fd);
void handle_signal(int sig);

#endif /* __PROCMAN_H__ */


