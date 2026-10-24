// expect: rejected utf16_char_lit_too_long @ L9
// c-only: C++ refuses u'\U0001F600' with a hard error #1535 (no_char16_t_representation) of its own; only C needs #3255 made an error
// twin: tests/scalar/plain_char.c
// Pins: 0023 item 8 (Q33): in C a UTF-16 literal whose value needs two code units is truncated; EDG's #3255 is an error in verify mode
#include <cerastes.h>

int utf16_truncated(void)
{
  return u'\U0001F600';
}
