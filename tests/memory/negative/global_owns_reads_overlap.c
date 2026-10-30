// expect: error permission @ L10
// mode: c23, c++20
// twin: tests/memory/global_owns_call.c
// Pins: 0041 item 7 (0039 item 7): owns(g) && reads(g) name one cell: the later atom's overlap assert fails in the contract's well-formedness, at the clause
#include <cerastes.h>

static int g = 3;

void f(void)
    CERASTES_REQUIRES(cerastes_owns(g) && cerastes_reads(g))
{
}

int main(void)
{
  return g - g;
}
