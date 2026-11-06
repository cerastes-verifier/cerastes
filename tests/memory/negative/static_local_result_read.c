// expect: error permission @ L18
// mode: c23, c++20
// twin: tests/memory/static_local_result.c
// Pins: 0052 item 2 (question 157): a covered live(r) gives the equality of liveness cells only, never access: reading *p after ensures(r, live(r)) is permission at the read (the contract has to say owns(*r) or reads(*r))
// Pre-registered from record 0052 item 2: permission at the read
#include <cerastes.h>

int *where(void)
    CERASTES_ENSURES(r, cerastes_live(r))
{
  static int s = 5;
  return &s;
}

int main(void)
{
  int *p = where();
  int v = *p;  // <- here
  return v;
}
