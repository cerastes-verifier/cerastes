// expect: error permission @ L12
// mode: c23, c++20
// twin: tests/memory/global_pointer_global_deref_atom.c
// Pins: 0045 items 3, 4: owns(*gp) without owns(gp): permission at the clause
#include <cerastes.h>
#include <stddef.h>

static int h = 7;
static int *gp = &h;

void f(void)
    CERASTES_REQUIRES(cerastes_owns(*gp))  // <- here
{
}

int main(void)
{
  return gp == &h ? 0 : 1;
}
