// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0043 items 2, 3: a self-referential struct node * member is a pointer cell; it is stored and compared, and n->next->v reads the pointee the function owns
#include <cerastes.h>
#include <stddef.h>

struct node {
  int v;
  struct node *next;
};

void init(struct node *n CERASTES_MUST_INIT, int v)
    CERASTES_REQUIRES(cerastes_owns(*n))
    CERASTES_ENSURES(cerastes_owns(*n) && n->v == v && n->next == NULL)
{
  n->v = v;
  n->next = NULL;
}

void link(struct node *a, struct node *b)
    CERASTES_REQUIRES(cerastes_owns(*a) && cerastes_live(b))
    CERASTES_ENSURES(cerastes_owns(*a) && a->next == b && a->v == cerastes_old(a->v) && cerastes_live(b))
{
  a->next = b;
}

int follow(struct node *n, struct node *m)
    CERASTES_REQUIRES(cerastes_owns(*n) && cerastes_owns(*m))
    CERASTES_ENSURES(r, cerastes_owns(*n) && cerastes_owns(*m) && r == m->v)
{
  n->next = m;
  return n->next->v;
}

int use_list(void)
    CERASTES_ENSURES(r, r == 3)
{
  struct node a;
  struct node b;
  init(&a, 1);
  init(&b, 2);
  link(&a, &b);
  CERASTES_CHECK(a.next == &b);
  return a.v + b.v;
}
