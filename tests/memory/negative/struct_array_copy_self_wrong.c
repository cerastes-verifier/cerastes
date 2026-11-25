// expect: error postcondition @ L19
// mode: c23, c++20
// twin: tests/memory/struct_array_copy_pointers.c
// Pins: 0063 item 13: *p = *p changes nothing: an ensures that wants p->a[1] one larger fails (postcondition at the closing brace)
// Pre-registered from record 0063 item 13 (Part E), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

void self_copy(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && p->a[1] < 1000)
    CERASTES_ENSURES(cerastes_owns(*p) && p->a[1] == cerastes_old(p->a[1]) + 1)
{
  *p = *p;
}  // <- here
