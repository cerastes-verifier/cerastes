// expect: rejected cerastes-unsupported_declaration @ L9
// expect: rejected cerastes-unsupported_type @ L9
// mode: c23, c++20
// twin: tests/memory/array_param_owns_bound.c
// Pins: 0060 item 15 (audit round 1, N11; e1e): an array of arrays as a parameter (`int a[][3]`, adjusted to a pointer to an array) is rejected explicitly, naming e1e; the older rule that rejects the adjusted pointer to an array (unsupported_type) reports the same line too
// Pre-registered from record 0060 item 15 (Part E), before the code
#include <cerastes.h>

int f(int a[][3])
{
  (void)a;
  return 0;
}
