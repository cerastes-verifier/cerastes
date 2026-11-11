// expect: rejected cerastes-char_subscripts @ L12
// mode: c23, c++20
// twin: tests/memory/array_index_unsigned_char.c
// Pins: 0059 item 4 (audit round 1, item 6): an index of plain char type is rejected like the compilers (-Wchar-subscripts at -Wall in clang and GCC): where char is signed a negative value indexes before the array; signed char and unsigned char indexes are accepted (the twin)
// Pre-registered from record 0059 item 4 as updated by audit round 1
#include <cerastes.h>

int pick(char c)
    CERASTES_REQUIRES(0 <= c && c < 3)
{
  int a[3] = {1, 2, 3};
  return a[c];
}
