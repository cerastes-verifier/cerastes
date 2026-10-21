// expect: rejected attr_must_also_appear_in_first_declaration @ L9
// mode: c23, c++20
// twin: tests/scalar/noreturn_abort.c
// Pins: 0015 item 5: a user cannot mark a library function noreturn afterwards: [[noreturn]] must be on the first declaration
// The front end rejects it (both modes); the library's abs stays an ordinary function.
#include <stdlib.h>
#include <cerastes.h>

[[noreturn]] int abs(int x);

int use(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  return abs(x);
}
