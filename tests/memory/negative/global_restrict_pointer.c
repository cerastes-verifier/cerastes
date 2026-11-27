// expect: rejected cerastes-unsupported_type @ L13
// mode: c23, c++20
// twin: tests/memory/global_pointer_declared_unused.c
// Pins: 0041 item 2 (0032 item 2): a restrict-qualified pointer global is rejected (restrict in C, __restrict in C++; M8) [0064 item 19: unchanged: restrict on a local, a global, a member, a pointee, a result or C++'s __restrict stays unsupported_type; only a parameter's restrict is item 19's rejection]
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
