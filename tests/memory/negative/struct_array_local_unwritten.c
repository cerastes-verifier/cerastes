// expect: error uninitialized @ L19
// mode: c23, c++20
// twin: tests/memory/struct_array_local.c
// Pins: 0063 items 5 and 6: s.a[2] was never written: its flag is false and the read fails uninitialized (a structure local without an initializer starts with every flag false)
// Pre-registered from record 0063 item 5 (Part C), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int g(void)
{
  struct S s;
  s.n = 3;
  s.a[1] = 4;
  return s.a[2] + s.n;  // <- here
}
