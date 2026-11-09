// expect: rejected cerastes-unsupported_annotation @ L14
// mode: c23, c++20
// verify-options: --strict-init
// twin: tests/memory/marked_global_strict.c
// Pins: 0054 item 5 and 0047 item 8: the same program without the marker: under --strict-init an unmarked variable that receives malloc's result is rejected (as reject_strict_malloc_assign_unmarked.c for a local)
// Pre-registered from record 0047 item 8 and 0054 item 5: rejected unsupported_annotation at the malloc call (the tag of the local analogue)
#include <cerastes.h>
#include <stdlib.h>

int *g = NULL;

int main(void)
{
  g = (int *)malloc(sizeof(int));  // <- here
  if (g == NULL) {
    return 1;
  }
  *g = 3;
  free(g);
  g = NULL;
  return 0;
}
