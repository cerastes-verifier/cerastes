// expect: rejected cerastes-unsupported_type @ L13
// mode: c23, c++20
// twin: tests/memory/global_pointer_declared_unused.c
// Pins: 0041 item 2 (0032 item 2): a restrict-qualified pointer global is rejected (restrict in C, __restrict in C++; M8)
#include <cerastes.h>

#ifdef __cplusplus
#define RESTRICT __restrict
#else
#define RESTRICT restrict
#endif

int *RESTRICT rp;

int ident(int a)
{
  return a;
}
