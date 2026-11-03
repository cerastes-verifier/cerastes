// expect: verified
// expect: native-ok
// mode: c23, c++20
// Port of the struct half of VeriFast's examples/globals.c (38 lines; ~/Documents/verifast at 0fb63506) to cerastes (decision 0045, M4.5c2b); the scalar half is tests/memory/verifast/globals.c (decision 0041).
// Adaptation: `//@ requires x |-> 7 &*& c |-> ?ctr &*& ctr->f |-> ?v; ensures x |-> 8 &*& c |-> ctr &*& ctr->f |-> v + 1;` becomes `cerastes_owns(x) && x == 7 && cerastes_owns(c) && cerastes_owns(c->f)` and `cerastes_owns(x) && x == 8 && cerastes_owns(c) && c == cerastes_old(c) && cerastes_owns(c->f) && c->f == cerastes_old(c->f) + 1` (the ghosts `ctr` and `v` are the tie `c == old(c)` and `old(c->f)`), with `c->f < 1000` added because cerastes checks the addition for overflow; `void m()` is `m(void)`; main: `malloc(sizeof(struct counter))` is the address of a static `struct counter obj` (malloc is M4.5d2) and `if (ctr == 0) abort();` goes with it, `//@ : main_full(globals)`, `open_module()` and the contract are gone, `assert` is CERASTES_CHECK, `int main()` is `int main(void)`, and `return 0;` is `return x0 + ctr_f - 43;` (0) because the ordinary compilers warn about locals that only an annotation reads (the file is `native-ok`).
// Left out: `free(ctr)` and the final `//@ leak ...` (malloc, free and leaks are M4.5d2); the two halves are separate ports, so the scalar `x` is initialized in each.
// Line map (this file : VeriFast's):  10 : 3   12 : 5   13 : 6   14 : 7   16 : 9   18 : 11   19 : 12   20 : 13   21 : 14   22 : 15   23 : 16   24 : 17   25 : 18   27 : 28   29 : 20   30 : 23   31 : 25   32 : 26   33 : 27   34 : 28   35 : 30   36 : 31   37 : 32   38 : 33   39 : 34   40 : 37   41 : 38
#include <cerastes.h>

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

static struct counter obj;

int main(void)
{
    int x0 = x;
    CERASTES_CHECK(x0 == 0);
    x = 7;
    struct counter *ctr = &obj;
    ctr->f = 42;
    c = ctr;
    m();
    int ctr_f = ctr->f;
    CERASTES_CHECK(ctr_f == 43);
    return x0 + ctr_f - 43;
}
