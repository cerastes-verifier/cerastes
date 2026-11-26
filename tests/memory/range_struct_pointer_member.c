// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 7 (audit round C1, finding 4; the lint's half is the well-formedness row of a pointer-to-structure value): a range over structures with a pointer-to-structure member (struct N { struct N *next; int v; } under owns_range(p, n)): link stores p[0].next = &p[1] and p[1].next = NULL and the caller passes a local array of two; follow reads the member through the loaded pointer (the pointee is the cell of p[1], known to be &p[1]); and, older than C1, a scalar range over struct P ** (M4.5e1b): clear stores NULL in the first pointer
// Pre-registered from record 0064 item 7 and the audit's finding 4 (g06, g05, z09 with its right postcondition), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

struct N {
  struct N *next;
  int v;
};

struct P {
  int x;
  int y;
};

int link(struct N *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 1)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && r == 0)
{
  (void)n;
  p[0].next = &p[1];
  p[1].next = NULL;
  return 0;
}

int use_link(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct N v[2] = {{NULL, 1}, {NULL, 2}};
  return link(v, 2);
}

int follow(struct N *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 1)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && r == 6)
{
  (void)n;
  p[0].next = &p[1];
  p[1].v = 5;
  struct N *q = p[0].next;
  p[1].v = 6;
  return q->v;
}

int clear(struct P **pp, int n)
    CERASTES_REQUIRES(cerastes_owns_range(pp, n) && n > 0)
    CERASTES_ENSURES(r, cerastes_owns_range(pp, n) && r == 0)
{
  (void)n;
  pp[0] = NULL;
  return 0;
}
