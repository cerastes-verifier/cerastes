// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 5 (Part B): &v[i] with a symbolic index given to a callee that owns the element and read back; &v[i].x stored in an int * and written through; &v[3] formed one past the end and never accessed (a value position allows i == N, as array_one_past.c has it for scalars)
// Pre-registered from record 0064 item 5 (Part B), before any run: verifies
#include <cerastes.h>

struct P {
  int x;
  int y;
};

void set(struct P *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && p->x == 7)
{
  p->x = 7;
}

int elem(int i)
    CERASTES_REQUIRES(0 <= i && i < 3)
    CERASTES_ENSURES(r, r == 7)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  set(&v[i]);
  return v[i].x;
}

int member(int i)
    CERASTES_REQUIRES(0 <= i && i < 3)
    CERASTES_ENSURES(r, r == 8)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  int *q = &v[i].x;
  *q = 8;
  CERASTES_CHECK(v[i].x == 8);
  return *q;
}

int past(void)
    CERASTES_ENSURES(r, r == 3)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  [[maybe_unused]] struct P *e = &v[3];
  return v[1].x;
}
