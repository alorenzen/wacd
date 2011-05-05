/*
 * A silly clock.  The second-hand swoops round at last moment
 */
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
  int now,i;
  wac_init(argc>1?argv[1]:"localhost");
  wac_stop();
  t(&now);
  wac_set(now);
  wac_moment(100);
  for (i = 0; i < 10; i++) {
    sleep(54);
    t(&now);
    wac_goto(now);
  }
  wac_finish();
}
