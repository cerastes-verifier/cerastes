// expect: rejected leading_character_ignored_in_char_literal @ L9
// mode: c23, c++20
// twin: tests/scalar/plain_char.c
// Pins: 0023 item 8 (Q33): 'abcde' drops its leading characters; EDG's #1654 is an error in verify mode
#include <cerastes.h>

int too_many(void)
{
  return 'abcde';
}
