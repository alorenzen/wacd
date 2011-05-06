/*
 * As you drink Dos Equis, time seems to move faster and faster.
 *
 * Andrew and Antal
 * 
 */
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include "wac.h"

void t(int *now)
{
  *now = time(0) - 4*60*60;
  *now = *now % (12*60*60);
}

int main(int argc, char**argv)
{
  int now,i,j, moment;
  wac_init(argc>1?argv[1]:"localhost"); /* I don't always drink in lab... */
  wac_stop();
  t(&now);
  wac_set(now); /* ...but when I do... */
  for (i = 0; i < 10; i++) {
    moment = 10000;
    for(j = 0; j < 3; j++) {
      wac_moment(moment);
      wac_go(); /* ...I prefer Dos Equis. */
      sleep(20);
      moment /= 10;
    }
  }
  wac_finish(); /* Happy Cinco de Mayo */
}
