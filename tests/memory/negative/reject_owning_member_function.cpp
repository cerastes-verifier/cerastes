// expect: rejected cerastes-unsupported_declaration @ L7
// expect: rejected cerastes-unsupported_annotation @ L9
// twin: tests/memory/heap_owning_result.c
// Pins: 0047 item 5 (audit S5; owner question 126): a struct with a member function is rejected (unsupported_declaration, M7), and [[cerastes::owning]] on a member function is rejected too (L9; owner questions 126, 127)
// Pre-registered from record 0047 item 5: rejected unsupported_annotation at the declaration; re-registered after audit round 1 (record 0047 item 5): audit S5: every other routine entity registration lets the attribute reach is rejected
#include <cerastes.h>
struct S {  // <- here
  int x;
  CERASTES_OWNING int *make();
};
