// expect: error postcondition @ L12
// mode: c23, c++20
// twin: tests/memory/not_invalidating_const.c
// Pins: 0058 item 2 (question 203): the const form with a body that returns *p + 1 where the ensures says r == *p: the marker on a pointer to const accepts the read share and checks the written value clause as usual, one postcondition error at the return
// Pre-registered from record 0058 item 2: one postcondition error at the return (the body's *p + 1 cannot overflow here: the requires bounds *p)
#include <cerastes.h>

int read_it(const int *p CERASTES_NOT_INVALIDATING)
    CERASTES_REQUIRES(cerastes_reads(*p) && *p < 100)
    CERASTES_ENSURES(r, cerastes_reads(*p) && r == *p)
{
  return *p + 1;  // <- here
}

int use(void)
    CERASTES_ENSURES(r, r == 4)
{
  int x = 4;
  int v = read_it(&x);
  x = 5;
  return v;
}
