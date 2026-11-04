// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pre-registered from record 0047 items 4, 6, 7 (VeriFast's globals.c with malloc and free): verifies
// Port of VeriFast's examples/globals.c with its `malloc` and `free` (~/Documents/verifast at 0fb63506) to cerastes (decision 0047, M4.5d2a); tests/memory/verifast/globals_struct.c (decision 0045) is the same program with the address of a static `struct counter obj` in place of `malloc` and no `free`.
// Adaptation: as in globals_struct.c (`requires x |-> 7 &*& c |-> ?ctr &*& ctr->f |-> ?v` is `cerastes_owns(x) && x == 7 && cerastes_owns(c) && cerastes_owns(c->f)` with `c->f < 1000` added for the overflow check, `ensures` the same with `c == cerastes_old(c)` and `c->f == cerastes_old(c->f) + 1`; `void m()` is `(void)`; main's contract, `open_module()` and `//@ : main_full(globals)` are gone, `assert` is CERASTES_CHECK, `return 0;` is `return x0 + ctr_f - 43;`); `#include "stdlib.h"` is `<stdlib.h>`; `malloc(sizeof(struct counter))` is converted by a C cast and `ctr == 0` is `ctr == NULL`.
// Left out: the final `//@ leak integer(&x, _) &*& pointer(&c, _);`: a global is no allocation (it has no token), so nothing is left to leak; the stale value of `c` after `free(ctr)` is never used (decision 0047 item 7 leaves blocks reachable from globals to d2b).
// Line map (this file : VeriFast's):  12 : 3   14 : 5   18 : 9   20 : 11   24 : 15   25 : 16   26 : 17   29 : 20   31 : 25   32 : 26   33 : 27   34 : 28   35 : 29   36 : 30   37 : 31   38 : 32   39 : 33   40 : 34   41 : 35   42 : 37
#include <cerastes.h>
#include <stdlib.h>

static int x;

struct counter {
    int f;
};

static struct counter *c;

void m(void)
    CERASTES_REQUIRES(cerastes_owns(x) && x == 7 && cerastes_owns(c) && cerastes_owns(c->f) && c->f < 1000)
    CERASTES_ENSURES(cerastes_owns(x) && x == 8 && cerastes_owns(c) && c == cerastes_old(c) && cerastes_owns(c->f) && c->f == cerastes_old(c->f) + 1)
{
    int y = x;
    x = y + 1;
    c->f = c->f + 1;
}

int main(void)
{
    int x0 = x;
    CERASTES_CHECK(x0 == 0);
    x = 7;
    struct counter *ctr = (struct counter *)malloc(sizeof(struct counter));
    if (ctr == NULL) abort();
    ctr->f = 42;
    c = ctr;
    m();
    int ctr_f = ctr->f;
    CERASTES_CHECK(ctr_f == 43);
    free(ctr);
    return x0 + ctr_f - 43;
}
