// expect: rejected cerastes-unsupported_annotation @ L9
// twin: tests/memory/init_must_init.c
// c-only: a redeclaration of a system function; glibc's C++ declarations carry an exception specification that conflicts; `time`, not glibc's C23 `strtol` (an asm label, rejected first by 0046 item 8)
// Pins: 0042 item 4 (audit S5): a marker on a user redeclaration of a function first declared in a system header is never silently ignored
#include <cerastes.h>

#include <time.h>

time_t time(time_t *timer CERASTES_MUST_INIT);  // <- here

int main(void)
{
  return 0;
}
