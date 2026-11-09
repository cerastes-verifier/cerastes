// expect: error permission @ L14
// mode: c23, c++20
// twin: tests/memory/owning_global_no_heap.c
// Pins: 0054 item 5 ("Stores", the value rule) and audit round 2 (Parts C and D), probe a11: an owning global receives the address of a local in a program with no allocation: not the start of a block the function holds
// Pre-registered from record 0054 item 5 and audit probe a11 (exit 3 "undeclared field tok" when written): permission at the assignment
#include <stddef.h>
#include <cerastes.h>

CERASTES_OWNING static int *g = NULL;

int main(void)
{
  int x = 0;
  g = &x;  // <- here
  return g == NULL;
}
