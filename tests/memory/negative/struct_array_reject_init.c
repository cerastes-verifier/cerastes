// expect: rejected cerastes-unsupported_expression @ L16
// mode: c23, c++20
// twin: tests/memory/struct_array_init.c
// Pins: 0063 item 6: a list that elides the array member's braces ({1, 2, 3, 4, 5}) is rejected like the compilers' -Wmissing-braces (told from {{1, 2, 3, 4}, 5} by the flag explicit_braces_on_aggregate, P82 (c))
// Pre-registered from record 0063 item 6 (Part C), before any run: rejected (tag from edg-backend's plan C3; to confirm at the joined run)
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int f(void)
{
  struct S s = {1, 2, 3, 4, 5};  // <- here
  return s.n;
}
