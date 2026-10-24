// expect: rejected multi_char_literal @ L9
// mode: c23, c++20
// twin: tests/scalar/plain_char.c
// Pins: 0023 item 8 (Q33): a multi-character literal has an implementation-defined value; EDG's #1422 is an error in verify mode
#include <cerastes.h>

int multi(void)
{
  return 'ab';
}
