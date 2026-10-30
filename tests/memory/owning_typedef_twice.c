// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 2 (re-audit of audit round 2's B1): a typedef'd function type used for two declarations of f, then a definition with the same owning: the declarations agree
#include <cerastes.h>
#include <stddef.h>

typedef void fn_t(CERASTES_OWNING int *p);
fn_t f;
fn_t f;

void f(CERASTES_OWNING int *p)
{
  if (p != NULL) {
    *p = 1;
  }
}

int g(void)
{
  int x = 0;
  f(&x);
  f(NULL);
  x = 2;
  return x;
}
