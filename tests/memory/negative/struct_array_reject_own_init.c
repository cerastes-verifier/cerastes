// expect: rejected cerastes-unsupported_declaration @ L16
// mode: c23, c++20
// twin: tests/memory/struct_array_init.c
// Pins: 0063 item 6: an initializer element that reads an element of the structure being initialized is rejected (the elements are stored after the zero fill, and the structure's own cells are not yet initialized; as array_reject_self_reading_init.c for a local array; verify mode passes --no_use_before_set_warnings, so no #549; to confirm at the joined run)
// Pre-registered from record 0063 item 6 (Part C), before any run: rejected (tag from edg-backend's plan C2: a local variable used in its own initializer)
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int f(void)
{
  struct S s = {{1, s.a[0]}, 2};  // <- here
  return s.n;
}
