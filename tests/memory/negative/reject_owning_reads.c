// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// twin: tests/memory/owning_param.c
// Pins: 0040 item 3 (question 75): reads(*p) beside owning is rejected (never lifted)
#include <cerastes.h>
#include <stddef.h>

void f(CERASTES_OWNING int *p) CERASTES_REQUIRES(cerastes_reads(*p)) CERASTES_ENSURES(cerastes_reads(*p)) { (void)p; }
