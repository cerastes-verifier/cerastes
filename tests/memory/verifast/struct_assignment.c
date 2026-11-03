// expect: verified
// expect: native-ok
// mode: c23, c++20
// Port of VeriFast's examples/struct_assignment.c (~/Documents/verifast at 0fb63506) to cerastes (decision 0044, M4.5c2a): copies of structures and nested structures (0043 item 7), by-value parameters and results, compound literals as arguments and results (the literals need the encoder of commit 5; the expectation stays `verified`).
// Adaptation: VeriFast's `test`, `test2`, `test3`, `test4`, `test5`, `test6` keep their names except `test` is `test1`; `void test()` is `int test1(void)` (and `test2`, `test3`) returning `a.x + r.p.x`, because the ordinary compilers warn about structs that only annotations read (the file is `native-ok`); the empty `//@ requires true; ensures true;` are absent; every `//@ assert a.x == 1;` is `CERASTES_CHECK(a.x == 1);` and the points-to asserts of `test2` (`r.q.x |-> 1 &*& r.q.y |-> 2`) are the same facts as values (`r.q.x == 1 && r.q.y == 2`); `test4(struct foo f)` returns `f.x` (an unused parameter is a compiler warning): `ensures r == f.x`; `//@ requires f.x < f.y;` is `CERASTES_REQUIRES(f.x < f.y)`; `result == (struct foo) { 40, 50 }` is `r.x == 40 && r.y == 50` (C has no `==` on structures, 0044 item 5), and test6's `(struct foo) { .y = 30, .x = 20 }` is written in declaration order (`.x = 20, .y = 30`; out-of-order C designators are rejected, 0043 item 6); the `//@ { &a; &b; &r; &t; }` ghost statements are absent (every struct local is a block here).
// Left out: the predicates `foo_ok`, `foo` and `bar` (M4.5f1).
// Line map (this file : VeriFast's):  10 : 1   15 : 6   5 : 11   22 : 15   25 : 18   28 : 21   36 : 29   39 : 33   42 : 36   48 : 42   51 : 45   56 : 48   64 : 59   76 : 74   84 : 79   105 : 103   112 : 108   115 : 112   116 : 113   119 : 116   122 : 120   123 : 121
#include <cerastes.h>

struct foo {
    int x;
    int y;
};

struct bar {
    struct foo p;
    struct foo q;
};

int test1(void)
{
    struct foo a = {1, 2};
    struct foo b = {3, 4};

    struct bar r = {{5, 6}, {7, 8}};
    struct bar t;

    t.p.x = 10;
    t.p.y = 20;
    t.q.x = 30;
    t.q.y = 40;

    CERASTES_CHECK(a.x == 1);
    CERASTES_CHECK(a.y == 2);

    a = b;
    CERASTES_CHECK(a.x == 3);
    CERASTES_CHECK(a.y == 4);
    r.p = r.q;
    CERASTES_CHECK(r.p.x == 7);
    CERASTES_CHECK(r.p.y == 8);
    r.q = t.p;
    CERASTES_CHECK(r.q.x == 10);
    CERASTES_CHECK(r.q.y == 20);
    a = r.p;
    CERASTES_CHECK(a.x == 7);
    CERASTES_CHECK(a.y == 8);
    t.p = b;
    CERASTES_CHECK(t.p.x == 3 && t.p.y == 4);

    r = t;
    CERASTES_CHECK(r.p.x == 3 && r.p.y == 4 && r.q.x == 30 && r.q.y == 40);
    return a.x + r.p.x;
}

int test2(void)
{
    struct foo a = {1, 2};
    struct foo b = {3, 4};

    struct bar r;
    struct bar t;

    r.q = a;
    CERASTES_CHECK(r.q.x == 1 && r.q.y == 2);
    t.p = b;
    CERASTES_CHECK(t.p.x == 3 && t.p.y == 4);
    a = b;
    CERASTES_CHECK(a.x == 3 && a.y == 4);
    r.p = r.q;
    CERASTES_CHECK(r.p.x == 1 && r.p.y == 2 && r.q.x == 1 && r.q.y == 2);
    r.q = t.p;
    CERASTES_CHECK(r.p.x == 1 && r.p.y == 2 && r.q.x == 3 && r.q.y == 4);
    a = r.p;
    CERASTES_CHECK(a.x == 1 && a.y == 2);
    t.q = b;
    CERASTES_CHECK(t.q.x == 3 && t.q.y == 4);

    r = t;
    CERASTES_CHECK(r.p.x == 3 && r.p.y == 4 && r.q.x == 3 && r.q.y == 4);
    return a.x + r.p.x;
}

int test3(void)
{
    struct foo a = {1, 2};
    struct foo b = {3, 4};

    struct bar r;
    struct bar t;

    r.q = a;
    t.p = b;
    t.q = a;
    a = b;
    r.p = r.q;
    r.q = t.p;
    a = r.p;
    t.p = b;

    r = t;
    return a.x + r.p.x;
}

int test4(struct foo f)
    CERASTES_REQUIRES(f.x < f.y)
    CERASTES_ENSURES(r, r == f.x)
{
    return f.x;
}

struct foo test5(void)
    CERASTES_ENSURES(r, r.x == 40 && r.y == 50)
{
    test4((struct foo){20, 30});
    return (struct foo){40, 50};
}

struct foo test6(void)
    CERASTES_ENSURES(r, r.x == 40 && r.y == 50)
{
    test4((struct foo){.x = 20, .y = 30});
    return (struct foo){.x = 40, .y = 50};
}
