// expect: error permission @ L18
// mode: c23, c++20
// twin: tests/memory/static_local_result.c
// Pins: 0052 item 2, audit 2 N1 (k_static_member.c), a documented false alarm lifted by M4.5d2b2: ensures(r, owns(*r)) of a function returning &s.x of a static struct: the implicit atom of a static struct is the whole structure's, and cover_of compares equal places, so the member is not covered and the function holds no separate owns(s.x): permission at the return, returns *r without holding it (a cover per leaf of an implicit struct atom is d2b2's)
// Pre-registered from audit round 2 (build/m4.5d2b/audit2.md) N1: permission at the return (pinned false alarm)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int *where(void)
    CERASTES_ENSURES(r, cerastes_owns(*r))
{
  static struct S s;
  s.x = 1;
  return &s.x;  // <- here
}

int main(void)
{
  int *p = where();
  *p = 2;
  return 0;
}
