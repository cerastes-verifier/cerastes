// expect: rejected cerastes-missing_decreases @ L10
// mode: c23, c++20
// twin: tests/scalar/noreturn_abort.c
// Pins: 0015 item 1 and 0013 item 3: a function that never returns by looping forever has a loop without a measure
// Stricter than C 6.8.5p6: every loop needs a decreases, and an endless one has none.
#include <cerastes.h>

[[noreturn]] void spin(void)
{
  for (;;) {
  }
}
