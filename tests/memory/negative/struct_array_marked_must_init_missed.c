// expect: error postcondition @ L23
// mode: c23, c++20
// twin: tests/memory/struct_array_marked_ref.c
// Pins: 0064 item 15 (294): a must_init filler that leaves a[3] unwritten returns without initializing every element, which must_init promises: the category is the open question 1 of report 0 (item 15 says uninitialized; the precedents negative/range_marker_must_init_partial.c and range_struct_marker_unwritten.c are postcondition at the closing brace, registered here)
// Pre-registered from the precedents range_marker_must_init_partial.c and range_struct_marker_unwritten.c (postcondition at the closing brace) against record 0064 item 15's wording (uninitialized): UNRESOLVED, a question to the orchestrator, before any run
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

void fill(struct S *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && p->a[0] == 1)
{
  p->a[0] = 1;
  p->a[1] = 2;
  p->a[2] = 3;
  p->n = 4;
}  // <- here
