// expect: rejected cerastes-unsupported_declaration @ L9
// mode: c23
// c-only: a variable length array parameter is C only (EDG's #411 rejects it in C++)
// twin: tests/memory/array_param_owns_bound.c
// Pins: 0060 item 15: a VLA parameter on a definition is rejected (C's VLA error before this session): the line is the function's declarator, pre-registered (the older diagnostic stood at the opening brace)
// Pre-registered from record 0060 item 15 (Part E), before the code
#include <cerastes.h>

int f(int n, int a[n])
{
  return n;
}
