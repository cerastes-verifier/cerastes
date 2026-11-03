// expect: error precondition @ L22
// mode: c23, c++20
// twin: tests/memory/verifast/struct_assignment.c
// Pins: 0044 items 3, 5 (the port's twin): test4((struct foo){30, 20}) violates f.x < f.y: precondition at the call (the by-value leaf's requires, the literal as the argument)
#include <cerastes.h>

struct foo {
    int x;
    int y;
};

int test4(struct foo f)
    CERASTES_REQUIRES(f.x < f.y)
    CERASTES_ENSURES(r, r == f.x)
{
    return f.x;
}

struct foo test5(void)
    CERASTES_ENSURES(r, r.x == 40 && r.y == 50)
{
    test4((struct foo){30, 20});  // <- here
    return (struct foo){40, 50};
}
