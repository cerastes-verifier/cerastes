// expect: verified
// expect: native-ok
// mode: c23, c++20
// A contract that owns the cell when c holds and only reads it otherwise.
// tag::source[]
#include <cerastes.h>
#include <stddef.h>

int step(int c, int *p)
    CERASTES_REQUIRES((c ? cerastes_owns(*p) : cerastes_reads(*p)) && *p >= 0 && *p < 100)
    CERASTES_ENSURES(r, (c ? cerastes_owns(*p) : cerastes_reads(*p)) && r == *p && r == cerastes_old(*p) + (c != 0 ? 1 : 0))
{
  if (c) {
    *p += 1;
  }
  return *p;
}

int use_step(int c)
    CERASTES_ENSURES(r, r == (c != 0 ? 2 : 1))
{
  int x = 1;
  return step(c, &x);
}
// end::source[]
