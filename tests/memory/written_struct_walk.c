// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 12 (question 99): owns(*q) for struct S *q as a written atom in an invariant, the loop walking two struct objects: one atom per leaf, the leaves of both objects the automatic places
#include <cerastes.h>
#include <stddef.h>
struct S {
  int a;
  int b;
};

int walk_s(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 1)
{
  struct S s1 = {1, 2};
  struct S s2 = {1, 3};
  struct S *q = &s1;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && s1.a == 1 && s2.a == 1 && (q == &s1 || q == &s2) && cerastes_owns(*q) && q->a == 1)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    
    q = (q == &s1) ? &s2 : &s1;
    i += 1;
  }
  return q->a;
}
