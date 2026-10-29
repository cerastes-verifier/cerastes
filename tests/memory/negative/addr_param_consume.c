// expect: error lifetime @ L14
// mode: c23, c++20
// twin: tests/memory/addr_param.c
// Pins: 0037 items 4 and 13: the parameter's block has the function body's scope exit, and a callee that keeps the cell fails it
#include <cerastes.h>
#include <stddef.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

void give_away(int a)
{
  dispose(&a);
}
