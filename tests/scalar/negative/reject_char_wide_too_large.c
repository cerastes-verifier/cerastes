// expect: rejected bad_character_value @ L10
// mode: c23, c++20
// twin: tests/scalar/char_literal.c
// Pins: 0024 item 5: L'\x100000000': the front end's #27 (bad_character_value, discretionary) is already an error in verify mode on this EDG build (observed with the pre-session binary); the test keeps it one
#include <cerastes.h>

int truncated(void)
    CERASTES_ENSURES(r, r >= 0)
{
  return (int)L'\x100000000';
}
