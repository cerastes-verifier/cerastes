// expect: error check @ L25
// mode: c23, c++20
// twin: tests/memory/struct_array_limit.c
// Pins: 0063 item 13: the 7-element by-value call returns 0: a check that it is 1 fails (the limit is vir::kMaxByValueArrayElements)
// Pre-registered from record 0063 item 13 (Part E), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

struct B {
  int a[7];
};

int last(struct B b)
    CERASTES_REQUIRES(b.a[6] == 0)
    CERASTES_ENSURES(r, r == 0)
{
  return b.a[6];
}

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct B b = {0};
  int v = last(b);
  CERASTES_CHECK(v == 1);  // <- here
  return v;
}
