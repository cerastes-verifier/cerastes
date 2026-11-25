// expect: verified
// expect: native-ok
// mode: c++20
// Pins: 0064 items 5, 6 and 16 (Part B): the C++ spellings of arrays of structures: cerastes::owns and cerastes::old over p[i].x with i == 0, a designated list that names every member, a partial list, an element through a pointer to the array's first element, and C++'s {0} over a one-member chain whose only members are an array (clang++ is silent: the elided subaggregate is the only member of its structure at every level), and a braced designated value whose 0 elides the braces of a structure that is the only member of its parent ({.t = {0}} over W1 {T1 t}, T1 {P1 p}: record item 16, "Designated lists": accepted in C++, a designator changes nothing; the chain has one int per level so that -Wextra's missing-field-initializers has nothing to flag; its -Wextra cell is not measured by a run, the measured two-member form omits a member)
// Pre-registered from record 0064 items 5, 6 and 16 (Part B), before any run: verifies
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct P1 {
  int x;
};

struct T1 {
  P1 p;
};

struct W1 {
  T1 t;
};

struct A1 {
  int a[4];
};

struct B1 {
  A1 x;
};

void set(P* p, int i)
    CERASTES_REQUIRES(cerastes::owns(*p) && i == 0)
    CERASTES_ENSURES(cerastes::owns(*p) && p->y == cerastes::old(p->y))
{
  p[i].x = 5;
}

int use(int i)
    CERASTES_REQUIRES(0 <= i && i < 3)
    CERASTES_ENSURES(r, r == 0)
{
  P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  P w[2] = {{.x = 7, .y = 8}};
  P* q = v;
  q[i].y = 9;
  CERASTES_CHECK(v[i].y == 9 && w[1].x == 0 && w[0].y == 8);
  set(&v[i], 0);
  return v[i].y - 9 + w[1].y;
}

int one_member()
    CERASTES_ENSURES(r, r == 0)
{
  B1 b = {0};
  CERASTES_CHECK(b.x.a[3] == 0);
  return b.x.a[1];
}

int designated_only_member()
    CERASTES_ENSURES(r, r == 0)
{
  W1 w = {.t = {0}};
  CERASTES_CHECK(w.t.p.x == 0);
  return w.t.p.x;
}
