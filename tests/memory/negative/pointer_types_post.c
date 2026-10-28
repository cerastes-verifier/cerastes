// expect: error postcondition @ L35
// mode: c23, c++20
// twin: tests/memory/pointer_types.c
// Pins: 0032 item 2: *pb is never set, so the sum is 15
#include <cerastes.h>

int widths(void)
    CERASTES_ENSURES(r, r == 16)
{
  short s = 0;
  unsigned char c = 0;
  long l = 0;
  long long ll = 0;
  unsigned u = 0;
  bool b = false;
  short *ps = &s;
  unsigned char *pc = &c;
  long *pl = &l;
  long long *pll = &ll;
  unsigned *pu = &u;
  bool *pb = &b;
  *ps = 1;
  *pc = 2;
  *pl = 3;
  *pll = 4;
  *pu = 5;
  int n = *ps;
  n += *pc;
  n += (int)*pl;
  n += (int)*pll;
  n += (int)*pu;
  if (*pb) {
    n += 1;
  }
  return n;
}
