// expect: rejected cerastes-pure_function @ L13
// mode: c23, c++20
// twin: tests/scalar/pure_check_small.c
// Pins: 0062 items 1 and 3: every parameter of a read pure function is a bool, an integer, an enumeration or a pointer to one of those or to a pointer (0061 item 3's types): a pointer to a structure is pure_function (structure members: M4.5f1 alone, record 0063 item 1's table; flip C of M4.5e1e1, expect line unchanged)
// Pre-registered from record 0062 (Part B), before the code: rejected. the tag and the line are question-160 candidates (the function's own line)
#include <cerastes.h>
#include <stddef.h>

struct S {
  int v;
};

CERASTES_PURE int get_v(const struct S *s)
    CERASTES_REQUIRES(s != NULL)
{
  return s->v;
}

int use(int y)
{
  return y;
}
