// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 12 (Part B): an element as a structure value: struct P t = v[i], v[j] = t, v[i] = v[j] with i == j possible (every source leaf is read first, then every destination leaf written, the flags set), and v[i] passed by value to a function whose precondition reads its members
// Pre-registered from record 0064 item 12 (Part B), before any run: verifies
#include <cerastes.h>

struct P {
  int x;
  int y;
};

int sum(struct P p)
    CERASTES_REQUIRES(0 < p.x && p.x < 100 && 0 < p.y && p.y < 100)
    CERASTES_ENSURES(r, r == p.x + p.y)
{
  return p.x + p.y;
}

int from_elem(int i)
    CERASTES_REQUIRES(0 <= i && i < 3)
    CERASTES_ENSURES(r, r == 0)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  struct P t = v[i];
  CERASTES_CHECK(t.x == v[i].x && t.y == v[i].y);
  return sum(t) - t.x - t.y;
}

int by_value(int i)
    CERASTES_REQUIRES(0 <= i && i < 3)
    CERASTES_ENSURES(r, 3 <= r && r <= 11)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  return sum(v[i]);
}

int to_elem(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 3 && 0 <= j && j < 3)
    CERASTES_ENSURES(r, r == 0)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  struct P t = {9, 8};
  v[j] = t;
  CERASTES_CHECK(v[j].x == 9 && v[j].y == 8);
  v[i] = v[j];
  CERASTES_CHECK(v[i].x == 9 && v[i].y == 8);
  return v[i].x - 9 + v[j].y - 8;
}
