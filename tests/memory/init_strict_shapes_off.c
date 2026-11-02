// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0042 item 8: the programs that --strict-init rejects (an unmarked local, an initialized object or a pointer local passed to a marked parameter) verify without the option: the option only rejects more
#include <cerastes.h>
#include <stddef.h>
void init(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 0)
{
  *p = 0;
}

int a(void)
    CERASTES_ENSURES(r, r == 0)
{
  int x = 5;
  init(&x);
  return x;
}

int b(void)
    CERASTES_ENSURES(r, r == 0)
{
  int x;
  x = 1;
  return x - 1;
}

int c(void)
    CERASTES_ENSURES(r, r == 0)
{
  int x;
  int *q = &x;
  init(q);
  return x;
}
