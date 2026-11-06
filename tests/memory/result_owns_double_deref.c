// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 2 (0045 item 1; the former pin reject_owns_result_double_deref.c): owns(**r) after owns(*r) in one ensures, both given (the callee keeps its requires atoms and hands the two cells back through the result); the caller reads **q; x and p are globals, so that no scope exit of the caller needs the liveness of a given place back (a given amount looks fresh to the caller)
// Pre-registered from record 0052 item 2: verifies in both modes (the least certain Part A positive: see the note on amounts)
#include <cerastes.h>

static int x = 4;
static int *p = &x;

int **hand(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp))
    CERASTES_ENSURES(r, cerastes_owns(*r) && cerastes_owns(**r) && r == pp && *r == cerastes_old(*pp) && **r == cerastes_old(**pp))
{
  return pp;
}

int main(void)
{
  int **q = hand(&p);
  int v = **q;
  CERASTES_CHECK(v == 4);
  return v - 4;
}
