// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 6 and 5 (Part B): braced initializers of a local array of structures: a nested list per element, a partial list (the elements it leaves out are zero, C's fill), {}, a designated list that names every member, a list of structures with a nested structure, non-constant elements (every element is evaluated before the first store), and sizeof(v) / sizeof(v[0]) of the named array (the staging pin array_reject_struct_elem_use.c's declarations are among them); one array per function (lists_a to lists_e): the five arrays in one function made Silicon time out at 300 s in c23 (question 328)
// Pre-registered from record 0064 items 5 and 6 (Part B), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

struct P {
  int x;
  int y;
};

struct Q {
  struct P t;
  int z;
};

int lists_a(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct P a[3] = {{1, 2}, {3, 4}, {5, 6}};
  CERASTES_CHECK(a[2].y == 6 && a[1].x == 3);
  return a[0].x - 1;
}

int lists_b(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct P b[3] = {{1, 2}};
  CERASTES_CHECK(b[0].y == 2 && b[1].x == 0 && b[1].y == 0 && b[2].x == 0);
  return b[0].x - 1;
}

int lists_c(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct P c[2] = {};
  CERASTES_CHECK(c[0].x == 0 && c[1].y == 0);
  return c[1].x;
}

int lists_d(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct P d[2] = {{.x = 7, .y = 8}, {.x = 9, .y = 10}};
  CERASTES_CHECK(d[1].x == 9 && d[0].y == 8);
  return d[1].y - 10;
}

int lists_e(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct Q e[2] = {{{1, 2}, 3}, {{4, 5}, 6}};
  CERASTES_CHECK(e[1].t.y == 5 && e[0].z == 3);
  return e[1].z - 6;
}

int dynamic(int a, int b)
    CERASTES_REQUIRES(0 <= a && a < 100 && 0 <= b && b < 100)
    CERASTES_ENSURES(r, r == 0)
{
  struct P v[2] = {{a, b}, {a + 1, 4}};
  CERASTES_CHECK(v[0].x == a && v[0].y == b && v[1].x == a + 1 && v[1].y == 4);
  return v[1].x - a - 1 + v[0].y - b;
}

int size(void)
    CERASTES_ENSURES(r, r == 3)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  CERASTES_CHECK(sizeof(v) == 3 * sizeof(struct P) && sizeof(v) / sizeof(v[0]) == 3);
  return (int)(sizeof(v) / sizeof(v[0])) + v[0].x - 1;
}
