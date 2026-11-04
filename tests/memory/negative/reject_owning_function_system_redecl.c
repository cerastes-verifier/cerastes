// expect: rejected cerastes-unsupported_annotation @ L9
// c-only: in C++ glibc's getenv has a noexcept that a redeclaration must repeat (decision 0046 item 6); C shows the rule on both platforms
// twin: tests/memory/heap_owning_result.c
// Pins: 0047 item 5 (audit S5; encode.cpp:2716's rule extended from parameter markers to routine markers): [[cerastes::owning]] on a redeclaration of a function a system header declares first
// Pre-registered from record 0047 item 5: rejected unsupported_annotation at the redeclaration; re-registered after audit round 1 (record 0047 item 5): audit S5: every other routine entity registration lets the attribute reach is rejected
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING char *getenv(const char *name);  // <- here
