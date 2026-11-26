// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 11 (Part B; audit round 1, note 15): object keys carry the index steps, so v[0].x = v[1].x++ (two constant elements), v[i].x = v[j].x++ under i != j (two elements) and v[i].x = v[i].y++ (two leaves of one element) are sequenced enough; a whole element passed by value beside a write to a leaf of another element, take(v[i], v[j].x++) under i != j, takes the path-precise row too (a whole-element read reads every leaf path of the element); no compiler warns on any of them
// Pre-registered from record 0064 item 11 (Part B), before any run: verifies
#include <cerastes.h>

struct P {
  int x;
  int y;
};

int consts(void)
    CERASTES_ENSURES(r, r == 3)
{
  struct P v[2] = {{1, 2}, {3, 4}};
  v[0].x = v[1].x++;
  CERASTES_CHECK(v[0].x == 3 && v[1].x == 4);
  return v[0].x;
}

int elements(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 2 && i != j)
    CERASTES_ENSURES(r, r == 0)
{
  struct P v[2] = {{1, 2}, {3, 4}};
  int old = v[j].x;
  v[i].x = v[j].x++;
  CERASTES_CHECK(v[i].x == old && v[j].x == old + 1);
  return v[i].x - old;
}

int leaves(int i)
    CERASTES_REQUIRES(0 <= i && i < 2)
    CERASTES_ENSURES(r, r == 0)
{
  struct P v[2] = {{1, 2}, {3, 4}};
  int old = v[i].y;
  v[i].x = v[i].y++;
  CERASTES_CHECK(v[i].x == old && v[i].y == old + 1);
  return v[i].x - old;
}

int take(struct P p, int d)
    CERASTES_REQUIRES(0 <= p.x && p.x < 100 && 0 <= d && d < 100)
    CERASTES_ENSURES(r, r == p.x + d)
{
  return p.x + d;
}

int whole(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 2 && i != j)
    CERASTES_ENSURES(r, 0 <= r && r <= 200)
{
  struct P v[2] = {{1, 2}, {3, 4}};
  return take(v[i], v[j].x++);
}
