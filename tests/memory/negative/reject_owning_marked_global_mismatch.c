// expect: rejected cerastes-annotation_mismatch @ L10
// mode: c23, c++20
// twin: tests/memory/owning_global_init_cleanup.c
// Pins: 0054 item 10 (P75 (a)): owning and ref_to_uninit on one variable are accepted together only when every declaration carries both; here neither declaration does, though the union holds both
// Pre-registered from record 0054 item 5 and 10: rejected annotation_mismatch at the later declaration (0006 item 3; naming both), ONE error as the in-progress encoder reports it (BEST READING: a second error at the first declaration, for ref_to_uninit, as reject_marker_mismatch_kind.c gives one per declaration, is possible; see the report)
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING int *g = NULL;
CERASTES_REF_TO_UNINIT extern int *g;  // <- here

int main(void)
{
  return g == NULL;
}
