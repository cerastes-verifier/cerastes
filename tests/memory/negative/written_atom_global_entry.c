// expect: error permission @ L15
// mode: c23, c++20
// twin: tests/memory/written_atom_global.c
// Pins: 0045 item 12: the function owns gp but not *gp: the atom's entry row fails (permission at the clause)
#include <cerastes.h>
#include <stddef.h>

static int h = 0;
static int *gp = &h;

void walk(int n)
    CERASTES_REQUIRES(cerastes_owns(gp) && 0 <= n && n <= 100)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_owns(*gp))  // <- here
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
  }
}

int main(void)
{
  return gp == NULL ? 1 : 0;
}
