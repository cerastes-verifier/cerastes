// expect: rejected utf16_char_lit_too_long @ L11
// mode: c23
// twin: tests/scalar/char_literal.c
// c-only: C++ refuses u'\x10000' with #1535 (no_char16_t_representation) instead
// Pins: 0024 item 5: u'\x10000': the front end's #27 (bad_character_value, discretionary) is already an error in verify mode on this EDG build (observed with the pre-session binary); the test keeps it one
#include <cerastes.h>

int truncated(void)
    CERASTES_ENSURES(r, r >= 0)
{
  return (int)u'\x10000';
}
