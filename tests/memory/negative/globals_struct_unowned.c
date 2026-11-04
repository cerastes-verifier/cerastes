// expect: error permission @ L21
// mode: c23, c++20
// twin: tests/memory/verifast/globals_struct.c
// Pins: 0045 item 3 and the port of VeriFast's examples/globals.c: m() does not own c->f (its contract names only x and c): the read c->f is a permission error
#include <cerastes.h>

// (the scalar x of the port is not part of this twin: it would give the pointed-to leaf a cell to alias)

struct counter {
    int f;
};

static struct counter *c;

void m(void)
    CERASTES_REQUIRES(cerastes_owns(c) && c != 0)
    CERASTES_ENSURES(cerastes_owns(c))
{
    int y = 1;
    if (y) { }
    c->f = c->f + 1;
}
