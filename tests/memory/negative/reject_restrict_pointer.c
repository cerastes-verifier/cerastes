// expect: rejected cerastes-unsupported_type @ L11
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: audit round 1 of M4.5b1a, 0032 item 2: a restrict-qualified pointer (restrict in C, __restrict in C++) is rejected, not verified (not lifted before M8) [0064 item 19: unchanged: restrict on a local, a global, a member, a pointee, a result or C++'s __restrict stays unsupported_type; only a parameter's restrict is item 19's rejection]
#include <cerastes.h>
#ifdef __cplusplus
#define RESTRICT __restrict
#else
#define RESTRICT restrict
#endif
int restrict_pointer(void) { int x = 0; int *RESTRICT p = &x; return *p; }
