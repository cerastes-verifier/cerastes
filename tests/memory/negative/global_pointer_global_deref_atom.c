// expect: rejected cerastes-unsupported_annotation @ L11
// mode: c23, c++20
// twin: tests/memory/global_pointer_global.c
// Pins: 0041 item 7 (0040 item 6, question 74): owns(*gp) names a place reached through a heap read: rejected until M4.5c
#include <cerastes.h>

static int h = 7;
static int *gp = &h;

void f(void)
    CERASTES_REQUIRES(cerastes_owns(gp) && cerastes_owns(*gp))
{
}

int main(void)
{
  return gp == &h ? 0 : 1;
}
