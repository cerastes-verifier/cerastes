// expect: error permission @ L27
// expect: error permission @ L27
// mode: c23, c++20
// twin: tests/memory/owning_field_global_unknown.c
// Pins: 0055 items 3, 11 (S7): main hands the global to f without assigning the leaf: its value is unknown, so main cannot show g.data == NULL || owns_block(g.data): "calls `f`, which needs to own the whole allocation `g.data` points to" (acc/ext2.c)
// Pre-registered from audit round 1 (record 0055 as revised, 78838ca), two permission errors at the call, as the frozen base reports with the clause by hand in both modes: "calls `f`, which needs to own the whole allocation `g.data` points to" (acc/ext2.c shows it) and "passes `g.data` and `g` to `f` ... they may be the same object"
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

struct buf g;

void f(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g) && g.data == NULL)
{
  free(g.data);
  g.data = NULL;
}

int main(void)
{
  f();  // <- here
  return 0;
}
