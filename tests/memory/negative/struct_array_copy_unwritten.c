// expect: error uninitialized @ L19
// mode: c23, c++20
// twin: tests/memory/struct_array_copy.c
// Pins: 0063 item 13 (question 296): the copy reads every element of the source: s.a was never written, so the copy `t = s` reports an element without a value (uninitialized at the copy)
// Pre-registered from record 0063 item 13 (Part E), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int f(void)
{
  struct S s;
  s.n = 5;
  struct S t;
  t = s;  // <- here
  return t.n;
}
