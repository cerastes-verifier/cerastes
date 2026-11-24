// expect: error uninitialized @ L19
// mode: c23, c++20
// twin: tests/memory/array_loop_fill.c
// Pins: 0059 item 8 ("What e1a cannot state"), 0061 item 9 (flip B, question 250): after a loop that fills an array declared without an initializer a read is uninitialized: only a quantified invariant says which elements were written; this program writes none, so it fails as before (a pinned false alarm); its verifying sibling is tests/memory/forall_fill_local.c (M4.5e1c Part C)
// Pre-registered from record 0059 item 8 (Part C), before the code
#include <cerastes.h>

int fill_then_read(void)
    CERASTES_ENSURES(r, r >= 0)
{
  int a[3];
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    a[i] = i;
    i = i + 1;
  }
  return a[1] * 0 + i;
}
