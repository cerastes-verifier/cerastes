// expect: verified
// expect: native-ok
// c-only: compound literal objects and restrict do not exist in C++ (decision 0044 items 7 and 11)
// Pins: 0044 item 7 (question 96): &(T){...} in C is a hidden local of the innermost block: allocated at its start, stored where evaluated, exited at its end; the shapes: (int){5}, a struct, as an argument, &(struct S){...}.y, a loop body, an if condition, a for init, a switch's controlling expression, an else arm, and distinct elements {x++, y++}
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void set(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && p->x == 7 && p->y == cerastes_old(p->y))
{
  p->x = 7;
}

int use_int(void)
    CERASTES_ENSURES(r, r == 6)
{
  int *p = &(int){5};
  *p += 1;
  return *p;
}

int use_struct(void)
    CERASTES_ENSURES(r, r == 2)
{
  struct S *p = &(struct S){1, 2};
  set(p);
  CERASTES_CHECK(p->x == 7);
  return p->y;
}

void direct(void)
{
  set(&(struct S){1, 2});
}

int member(void)
    CERASTES_ENSURES(r, r == 3)
{
  int *q = &(struct S){1, 2}.y;
  *q += 1;
  return *q;
}

void in_loop(void)
{
  int i = 0;
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    set(&(struct S){1, 2});
    i += 1;
  }
}

int in_if_condition(void)
{
  struct S *p = 0;
  if ((p = &(struct S){1, 2}) != 0) {
    return p->x;
  }
  return 0;
}

int in_for_init(void)
    CERASTES_ENSURES(r, r == 3)
{
  int s = 0;
  int *p = 0;
  CERASTES_INVARIANT(0 <= s && s <= 3)
  CERASTES_DECREASES(3 - s)
  for (p = &(int){1}; s < 3; s += 1) {
    *p = s;
  }
  return s;
}

int in_switch(void)
    CERASTES_ENSURES(r, r == 1)
{
  switch (*&(int){2}) {
  case 2:
    return 1;
  default:
    return 0;
  }
}

void in_else(int c)
{
  if (c != 0) {
    return;
  } else
    set(&(struct S){1, 2});
}

int distinct_elements(int x, int y)
    CERASTES_REQUIRES(0 <= x && x < 100 && 0 <= y && y < 100)
    CERASTES_ENSURES(r, r == y + 1)
{
  struct S *p = &(struct S){x++, y++};
  return p->y + 1 + (x - x);
}
