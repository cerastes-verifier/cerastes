// expect: rejected cerastes-unsupported_annotation @ L10
// mode: c23, c++20
// twin: tests/memory/global_owns_call.c
// Pins: 0041 item 7 (audit S4, 0037 item 5): owns(g) twice in one clause list would hold two full permissions and make the body vacuous: rejected (never lifted)
#include <cerastes.h>

static int g = 3;

void f(void)
    CERASTES_REQUIRES(cerastes_owns(g) && cerastes_owns(g))
{
}

int main(void)
{
  return g - g;
}
