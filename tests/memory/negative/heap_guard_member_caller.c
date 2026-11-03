// expect: error permission @ L21
// mode: c23, c++20
// twin: tests/memory/heap_guard_member.c
// Pins: 0045 item 11: the caller owns *p but not *p->q, and p->q may be non-null: permission at the call
#include <cerastes.h>
#include <stddef.h>
struct M { int y; };
struct L {
  struct M *q;
  int x;
};

void clear(struct L *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && (p->q == NULL || cerastes_owns(*p->q)))
    CERASTES_ENSURES(cerastes_owns(*p) && p->q == cerastes_old(p->q) && (p->q == NULL || cerastes_owns(*p->q)));

void use(struct L *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  clear(p);  // <- here
}
