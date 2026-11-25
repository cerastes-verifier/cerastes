// expect: error lifetime @ L22
// mode: c23, c++20
// twin: tests/memory/struct_array_live.c
// Pins: 0063 item 6: the callee takes the liveness of the structure's block and does not give it back: at the scope exit the address of an element of s.a may still be in use (lifetime, reported at the return)
// Pre-registered from record 0063 item 6 (Part C), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

void hold(int *p)
    CERASTES_REQUIRES(cerastes_live(p));

int f(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct S s = {{1, 2, 3, 4}, 5};
  hold(&s.a[1]);
  return s.n;  // <- here
}
