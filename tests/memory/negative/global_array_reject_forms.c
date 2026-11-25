// expect: rejected cerastes-unsupported_declaration @ L11
// expect: rejected cerastes-unsupported_declaration @ L13
// expect: rejected cerastes-unsupported_declaration @ L17
// mode: c23
// c-only: array designators are C only (in C++ c99_designator: the _cxx pins)
// twin: tests/memory/global_array_declared.c
// Pins: 0063 items 2 and 9 (0059 items 2 and 3): rejected like the compilers or by item 1's table (to confirm at the joined run, the tag of each): a tentative array definition without a bound (int g[]; clang and GCC warn, EDG completes it to one element silently: audit S6), an array designator (M8) and a block-scope extern array (M8)
// Pre-registered from record 0063 item 2 (Part B), before any run: rejected
#include <cerastes.h>

int unsized[];  // <- here

int des[3] = {[1] = 5};  // <- here

int f(void)
{
  [[maybe_unused]] extern int ext[3];  // <- here
  return 0;
}
