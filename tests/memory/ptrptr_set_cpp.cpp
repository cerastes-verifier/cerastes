// expect: verified
// expect: native-ok
// mode: c++20
// Pins: 0040 item 2: a T** parameter: owns(*pp) is a pointer cell's permission (WF in place of a range); the callee stores q, the caller learns *pp == q (C++ spellings)
#include <cerastes.h>
#include <stddef.h>

void set(int **pp, int *q)
    CERASTES_REQUIRES(cerastes::owns(*pp) && cerastes::live(q))
    CERASTES_ENSURES(cerastes::owns(*pp) && *pp == q && cerastes::live(q))
{
  *pp = q;
}

int use_set(void)
    CERASTES_ENSURES(r, r == 4)
{
  int x = 1;
  int y = 4;
  int *p = &x;
  set(&p, &y);
  return *p;
}
