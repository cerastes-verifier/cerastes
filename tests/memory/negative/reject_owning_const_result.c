// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/heap_owning_result.c
// Pins: 0047 item 5: [[cerastes::owning]] on a function that returns a pointer to a const object: such a block could never be freed without a cast that drops the qualifier (audit N1)
// Pre-registered from record 0047 item 5: rejected unsupported_annotation at the marked line; re-registered after audit round 1 (record 0047 item 5): audit round 2 N1
#include <cerastes.h>
#include <stddef.h>

CERASTES_OWNING const int *mk(void) { return 0; }  // <- here
