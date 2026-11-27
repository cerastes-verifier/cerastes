// expect: error check @ L22
// mode: c23, c++20
// twin: tests/memory/struct_array_heap_malloc.c
// Pins: 0064 item 15 (calloc: every element zero and set): a check that an element is 1 after calloc is false: check, at the check
// Pre-registered from record 0064 item 15, before any run: unmeasured
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

int zeroed(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct S *p = (struct S *)calloc(1, sizeof(struct S));
  if (p == NULL) {
    return -1;
  }
  CERASTES_CHECK(p->a[2] == 1);  // <- here
  free(p);
  return 0;
}
