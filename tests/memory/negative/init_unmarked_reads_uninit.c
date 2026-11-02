// expect: error uninitialized @ L14
// mode: c23, c++20
// twin: tests/memory/init_unmarked_flag_row.c
// Pins: 0042 item 7 (question 82): a reads parameter given an object without a value: uninitialized at the call
#include <cerastes.h>

int look(int *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
    CERASTES_ENSURES(r, cerastes_reads(*p) && r == *p);

int use_look(void)
{
  int x;
  return look(&x);  // <- here
}
