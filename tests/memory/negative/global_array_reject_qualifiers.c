// expect: rejected cerastes-unsupported_type @ L11
// expect: rejected cerastes-unsupported_type @ L12
// expect: rejected cerastes-unsupported_type @ L13
// mode: c23
// c-only: _Atomic is a keyword in C only
// twin: tests/memory/global_array_declared.c
// Pins: 0063 item 2 (0059 item 2's rule for a local, for a global): a const table (the const rule is tested before the array rule: M4.6), a volatile element (M8) and an _Atomic element (M8) are rejected at their declarations (the tag is unsupported_type as the local pins' and global_volatile.c's; to confirm at the joined run for the const table)
// Pre-registered from record 0063 item 2 (Part B), before any run: rejected
#include <cerastes.h>

const int table[3] = {1, 2, 3};  // <- here
volatile int vol[2];  // <- here
_Atomic int atom[2];  // <- here

int ident(int a)
{
  return a;
}
