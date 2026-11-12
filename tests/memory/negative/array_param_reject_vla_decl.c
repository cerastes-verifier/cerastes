// expect: rejected cerastes-unsupported_declaration @ L9
// mode: c23
// c-only: a variable length array parameter is C only (EDG's #411 rejects it in C++)
// twin: tests/memory/array_param_owns_bound.c
// Pins: 0060 item 15 (question 241, M8): a VLA parameter `int a[n]` on a declaration without a body is rejected (it was accepted before this session). C only: in C++ the front end rejects it
// Pre-registered from record 0060 item 15 (Part E), before the code
#include <cerastes.h>

int f(int n, int a[n])
    CERASTES_REQUIRES(n >= 0);

int g(void)
{
  int x[2] = {1, 2};
  return f(2, x);
}
