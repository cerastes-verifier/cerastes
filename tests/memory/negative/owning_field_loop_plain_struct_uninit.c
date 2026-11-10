// expect: error uninitialized @ L30
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_plain_struct_return.c
// Pins: 0056 item 11 (question 214): the same plain-struct loop with a member `s.y` that is never given a value and is read after the loop: uninitialized at the read, so the program that verifies is checked for its own values and not only for the absence of errors
// Pre-registered from the designed outcome (not probed before filing): uninitialized at the read `return s.y;`, in both modes; question 160 applies to the line
#include <cerastes.h>

struct pt {
  int x;
  int y;
};

int f(int n)
    CERASTES_REQUIRES(0 <= n && n <= 3)
{
  struct pt s;
  s.x = 1;
  int k = 0;
  int *pk = &k;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    *pk = i;
    if (i == n) {
      return 0;
    }
    i = i + 1;
  }
  return s.y;
}
