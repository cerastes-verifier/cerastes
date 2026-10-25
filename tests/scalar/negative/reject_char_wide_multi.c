// expect: rejected too_many_characters @ L9
// mode: c23, c++20
// twin: tests/scalar/plain_char.c
// Pins: 0023 item 8 (Q33): L'ab' keeps only its first character in C (EDG's #26 is a warning there, a hard error in C++); #26 is an error in verify mode
#include <cerastes.h>

int wide_multi(void)
{
  return L'ab';
}
