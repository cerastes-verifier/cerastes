// expect: verified
// expect: native-ok
// mode: c23
// c-only: a lone zero that elides the braces of a structure's first member is C's universal zero initializer; in C++ item 16's other rule rejects the shape (clang++ -Wmissing-braces)
// Pins: 0064 item 16 (audit round R, should-fix 3b; Commit 2): the literal spelling 0x0 of a lone zero stays accepted over the two parents the rule's other call sites read, and a zero over a structure whose first member is an array, the audit's struct A { int a[4]; int n; } (negative/struct_init_reject_nonliteral_zero_sites_c.c rejects '\0' there): an array member in a parent of two members (array_member, array_element) and a nested structure in one (nested_member). Every leaf is zero. clang's C and GCC are silent on each at -Wall -Wextra (the audit's q339 and q339c cells); the Mac's clang compile of this file is the check I could make, the box's gcc is the harness's
// Pre-registered from record 0064 item 16 (question 339, the rule's three call sites), before any run: verifies
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct A {
  int a[4];
  int n;
};

struct T {
  struct P p;
  int z;
};

int array_member(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct A t = {0x0};
  return t.n;
}

int array_element(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct A t = {0x0};
  return t.a[3];
}

int nested_member(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct T t = {0x0};
  return t.p.y;
}
