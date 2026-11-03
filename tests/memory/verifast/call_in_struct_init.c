// expect: verified
// expect: native-ok
// mode: c23, c++20
// Port of VeriFast's tests/call_in_struct_init.c (~/Documents/verifast at 0fb63506) to cerastes (decision 0044, M4.5c2a).
// Adaptation: `foo()` is `foo(void)`; `//@ requires true; ensures result.x == 1 && result.y == 2;` is `CERASTES_ENSURES(r, r.x == 1 && r.y == 2)` (`result` is the contract's own name `r`; the empty requires is absent); `//@ ensures true;` of `test` is absent; `assert(p.x == 1 && p.y == 2);` is `CERASTES_CHECK(...)`; `void test()` is `int test(void)` returning `p.x + p.y`, because the ordinary compilers warn about an initialized struct that only an annotation reads (the file is `native-ok`); the struct, `foo`'s body and the call that initializes `p` (a struct result stored into its own members, 0044 item 5) are as written.
// Left out: nothing.
// Line map (this file : VeriFast's):  10 : 1   15 : 6   5 : 8   18 : 10   19 : 11   5 : 14   24 : 18   5 : 19
#include <cerastes.h>

struct point {
    int x;
    int y;
};

struct point foo(void)
    CERASTES_ENSURES(r, r.x == 1 && r.y == 2)
{
    struct point p = {1, 2};
    return p;
}

int test(void)
{
    struct point p = foo();
    CERASTES_CHECK(p.x == 1 && p.y == 2);
    return p.x + p.y;
}
