// expect: error permission @ L15
// mode: c23, c++20
// twin: tests/memory/global_pointer_global.c
// Pins: 0041 items 6 and 7: the function owns the variable gp and knows it is not null, but owns nothing of its target (owns(*gp) is M4.5c): *gp is permission
#include <cerastes.h>
#include <stddef.h>

static int h = 7;
static int *gp = &h;

void set(void)
    CERASTES_REQUIRES(cerastes_owns(gp) && gp != NULL)
    CERASTES_ENSURES(cerastes_owns(gp))
{
  *gp = 1;
}
