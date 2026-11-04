// expect: error permission @ L20
// mode: c23, c++20
// twin: tests/memory/global_pointer_target.c
// Pins: 0045 item 6: *gp is h, which the call also passes as *p: one cell twice (permission at the call)
#include <cerastes.h>
#include <stddef.h>

static int h = 7;
static int *gp = &h;

void two(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(gp) && cerastes_owns(*gp))
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_owns(gp) && gp == cerastes_old(gp) && cerastes_owns(*gp))
{
  (void)p;
}

int main(void)
{
  two(&h);  // <- here
  return gp == NULL ? 1 : 0;
}
