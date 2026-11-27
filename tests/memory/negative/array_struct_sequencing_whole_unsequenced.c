// expect: error unsequenced @ L25
// mode: c23, c++20
// twin: tests/memory/array_struct_sequencing.c
// Pins: 0064 item 11 (audit round 1, note 15): the by-value argument v[i] reads every leaf path of the element, and v[j].x++ writes a leaf of an element of the same array: without i != j the two may reach one leaf, so the call takes the path-precise unsequenced row (at the later access, as the leaf-against-leaf twin has it)
// Pre-registered from the record's revised item 11 (Part B), before any run: the one error (the line is the call's, one statement)
#include <cerastes.h>

struct P {
  int x;
  int y;
};

int take(struct P p, int d)
    CERASTES_REQUIRES(0 <= p.x && p.x < 100 && 0 <= d && d < 100)
    CERASTES_ENSURES(r, r == p.x + d)
{
  return p.x + d;
}

int whole(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 2)
    CERASTES_ENSURES(r, 0 <= r && r <= 200)
{
  struct P v[2] = {{1, 2}, {3, 4}};
  return take(v[i], v[j].x++);  // <- here
}
