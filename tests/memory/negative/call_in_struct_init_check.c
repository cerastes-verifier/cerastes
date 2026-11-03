// expect: error check @ L22
// mode: c23, c++20
// twin: tests/memory/verifast/call_in_struct_init.c
// Pins: 0044 item 5 (the port's twin): foo's ensures gives p.y == 2, so check(p.y == 1) after `struct point p = foo();` fails
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
    CERASTES_CHECK(p.x == 1 && p.y == 1);  // <- here
    return p.x + p.y;
}
