// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pre-registered from record 0047 items 4, 6, 7 (VeriFast's swap.c main): verifies
// Port of VeriFast's examples/swap.c (all of it, with `main`; ~/Documents/verifast at 0fb63506) to cerastes (decision 0047, M4.5d2a); `swap` and `point_mirror` are tests/memory/verifast/swap.c's (decisions 0040, 0043), repeated so that `main` has its callees.
// Adaptation: `#include "stdlib.h"` is `<stdlib.h>`, `<stdbool.h>` is dropped (`bool` is a keyword in C23 and C++); `integer(a, ?x) &*& integer(b, ?y)` and `p->x |-> ?x &*& p->y |-> ?y` are the `cerastes_owns` clauses with `cerastes_old` as in swap.c; `malloc(sizeof(struct point))` is converted by a C cast, `p == 0` is `p == NULL`, `//@ : main` and the contract are gone, `assert(b)` is CERASTES_CHECK and `return 0;` is `return b ? 0 : 1;` (0) because the ordinary compilers warn about a local that only an annotation reads (the file is `native-ok`).
// Line map (this file : VeriFast's):  11 : 4   15 : 8   16 : 9   17 : 10   20 : 13   25 : 18   29 : 22   32 : 25   34 : 29   35 : 30   36 : 31   37 : 32   38 : 33   39 : 34   40 : 35   41 : 36   42 : 37
#include <cerastes.h>
#include <stdlib.h>

void swap(int *a, int *b)
    CERASTES_REQUIRES(cerastes_owns(*a) && cerastes_owns(*b))
    CERASTES_ENSURES(cerastes_owns(*a) && cerastes_owns(*b) && *a == cerastes_old(*b) && *b == cerastes_old(*a))
{
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

struct point {
    int x;
    int y;
};

void point_mirror(struct point *p)
    CERASTES_REQUIRES(cerastes_owns(p->x) && cerastes_owns(p->y))
    CERASTES_ENSURES(cerastes_owns(p->x) && cerastes_owns(p->y) && p->x == cerastes_old(p->y) && p->y == cerastes_old(p->x))
{
    swap(&p->x, &p->y);
}

int main(void)
{
    struct point *p = (struct point *)malloc(sizeof(struct point));
    if (p == NULL) { abort(); }
    p->x = 3;
    p->y = 10;
    point_mirror(p);
    bool b = p->x == 10 && p->y == 3;
    CERASTES_CHECK(b);
    free(p);
    return b ? 0 : 1;
}
