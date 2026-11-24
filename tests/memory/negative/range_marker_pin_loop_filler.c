// expect: error postcondition @ L18
// mode: c23, c++20
// twin: tests/memory/range_marker_must_init.c
// Pins: PIN of question 239 (0060 item 12; 0061 item 9, flip B, question 250): a must_init filler that fills with a loop cannot state, without a quantifier, that the elements below i are initialized, so the loop's end does not give back every flag: a false postcondition at the closing brace (the filler is correct); this program writes no quantified invariant, so it fails as before; its verifying sibling is tests/memory/forall_must_init_filler.c (M4.5e1c Part C)
// Pre-registered from record 0060 item 12 (Part E), before the code
#include <cerastes.h>
#include <stddef.h>

void fill(int *p CERASTES_MUST_INIT, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n))
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n) CERASTES_DECREASES(n - i) while (i < n) {
    p[i] = 0;
    i++;
  }
}
