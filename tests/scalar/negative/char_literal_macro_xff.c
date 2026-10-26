// expect: error conversion @ L14
// expect: error conversion @ L20
// mode: c23, c++20
// twin: tests/scalar/char_literal_macro.c
// Pins: 0024 item 5: char_literal_macro.c with the macro '\xff'. The error is at the use (an overflow through a macro is reported at the use, with a note at the definition, observed with the pre-session binary);
// each function is its own method, so each reports its use; the two uses in use_twice are one (category, line).
#include <cerastes.h>

#define DELETE_CHAR '\xff'

int delete_char(void)
    CERASTES_ENSURES(r, r == 127)
{
  return DELETE_CHAR;
}

int use_twice(void)
    CERASTES_ENSURES(r, r == 254)
{
  return DELETE_CHAR + DELETE_CHAR;
}
