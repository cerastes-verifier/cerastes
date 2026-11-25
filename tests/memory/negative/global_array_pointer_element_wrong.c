// expect: error check @ L13
// mode: c23, c++20
// twin: tests/memory/global_array_values.c
// Pins: 0063 item 9: a pointer element of the initial list: ptrs[0] is the address of x and ptrs[1] is null, so ptrs[1] == &x is not a fact
// Pre-registered from record 0063 item 9 (Part B), before any run: the one error
#include <cerastes.h>

static int x = 7;
static int *ptrs[2] = {&x, 0};

int main(void)
{
  CERASTES_CHECK(ptrs[1] == &x);  // <- here
  return ptrs[1] == 0 ? x - 7 : 1;
}
