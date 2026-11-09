// expect: rejected cerastes-annotation_mismatch @ L10
// mode: c23, c++20
// twin: tests/memory/owning_global_init_cleanup.c
// Pins: 0054 item 5 and 10 (P75 (a): EDG unions the attribute lists of the declarations of a variable without a diagnostic): a declaration of the global without owning beside one with it is annotation_mismatch, reported at the later declaration (0006 item 3; naming both)
// Pre-registered from record 0054 item 5: rejected annotation_mismatch at the later declaration
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING int *g = NULL;
extern int *g;  // <- here

int main(void)
{
  return g == NULL;
}
