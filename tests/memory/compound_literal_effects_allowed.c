// expect: verified
// expect: native-ok
// c-only: compound literals are C (C++ rejects them, record 0044 item 7)
// Pins: 0063 item 12 (audit round 2, B1, question 308): the three positions where an element of an array literal may write: a declaration's initializer (int *q = (int[]){x++, 2};), the literal as the whole full expression ((void)(int[]){x++, 2};; to confirm at the run that the void cast counts as the whole full expression), and an element that is a call (x = (int[]){bump(&x)}[0];: the call is sequenced and exempt)
// Pre-registered from audit round 2 of record 0063 (B1, question 308), before any run: verifies
#include <cerastes.h>

int bump(int *q)
    CERASTES_REQUIRES(cerastes_owns(*q) && *q < 100)
    CERASTES_ENSURES(r, cerastes_owns(*q) && *q == cerastes_old(*q) + 1 && r == cerastes_old(*q))
{
  int v = *q;
  *q = v + 1;
  return v;
}

int in_declaration(void)
    CERASTES_ENSURES(r, r == 3)
{
  int x = 1;
  int *q = (int[]){x++, 2};
  CERASTES_CHECK(x == 2 && q[0] == 1 && q[1] == 2);
  return q[0] + q[1];
}

int whole_expression(void)
    CERASTES_ENSURES(r, r == 2)
{
  int x = 1;
  (void)(int[]){x++, 2};
  return x;
}

int call_element(void)
    CERASTES_ENSURES(r, r == 1)
{
  int x = 1;
  x = (int[]){bump(&x)}[0];
  CERASTES_CHECK(x == 1);
  return x;
}
