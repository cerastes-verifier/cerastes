// expect: verified
// expect: native-ok
// c-only: in C++ a {0} whose 0 elides the braces of a nested structure or of an element is rejected like clang++'s -Wmissing-braces (record 0064 item 16, question 304; array_struct_reject_zero_cpp.cpp)
// Pins: 0064 item 16: C's lone {0} (both compilers are silent on it at every depth) zeroes a structure whose first member is a structure, an array of structures, an array of structures that begin with a structure, a structure that begins with a structure beginning with an array (a base defect fixed here: the frozen build rejected it in both modes), a compound literal and a global; and (function designated) an unbraced designated zero, {.p = 0} over a structure whose first member is a structure and {.s = 0} over one that begins with a structure beginning with an array (record item 16, "Designated lists": silent in both compilers at -Wall -Wextra in C, measured in s0/desig; C++ rejects both, array_struct_reject_designated_zero.cpp)
// Pre-registered from record 0064 item 16 (Part B), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

struct P {
  int x;
  int y;
};

struct T {
  struct P p;
  int z;
};

struct S {
  int a[4];
  int n;
};

struct U {
  struct S s;
  int z;
};

struct T g = {0};

int locals(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct T t = {0};
  struct P v[2] = {0};
  struct T tv[2] = {0};
  struct U u = {0};
  struct T w = (struct T){0};
  CERASTES_CHECK(t.p.x == 0 && t.p.y == 0 && t.z == 0);
  CERASTES_CHECK(v[1].x == 0 && v[0].y == 0);
  CERASTES_CHECK(tv[1].p.y == 0 && tv[0].z == 0);
  CERASTES_CHECK(u.s.a[3] == 0 && u.s.n == 0 && u.z == 0);
  CERASTES_CHECK(w.p.x == 0 && w.z == 0);
  return t.z + v[1].y + tv[0].p.x + u.z + w.z;
}

int designated(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct T t = {.p = 0};
  struct U u = {.s = 0};
  CERASTES_CHECK(t.p.x == 0 && t.p.y == 0 && t.z == 0);
  CERASTES_CHECK(u.s.a[3] == 0 && u.s.n == 0 && u.z == 0);
  return t.z + u.z;
}

int main(void)
{
  CERASTES_CHECK(g.p.x == 0 && g.p.y == 0 && g.z == 0);
  return g.z;
}
