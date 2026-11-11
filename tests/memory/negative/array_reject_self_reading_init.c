// expect: rejected cerastes-unsupported_declaration @ L11
// (no frontend-warning line: verify mode passes --no_use_before_set_warnings, so EDG's #549 is never printed)
// mode: c23, c++20
// twin: tests/memory/array_init_dynamic.c
// Pins: 0059 item 3 (P78 update): an element of the initializer that reads the array being declared (int a[2] = {1, a[0]};) is rejected, at the declaration (lifted by: not planned; the elements are stored in order and the array's own cells are not yet initialized)
// Pre-registered from the P78 update of record 0059 item 3
#include <cerastes.h>

int f(void)
{
  int a[2] = {1, a[0]};
  return a[1];
}
