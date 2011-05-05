/*
 * Implementation of a "standard" clock.
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
  t(&now);
  wac_set(now);
  wac_moment(500);
  wac_go();
  wac_finish();
}
