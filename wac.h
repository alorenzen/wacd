#ifndef WAC_H
#define WAC_H

#define DEFAULT_PORT 10010

int wac_init(char *hostname);
int wac_set(int secs);
int wac_get(void);
int wac_moment(int msecs);
int wac_go();
int wac_stop();
int wac_goto(int time);
int wac_finish();
int wac_shutdown();
#endif
