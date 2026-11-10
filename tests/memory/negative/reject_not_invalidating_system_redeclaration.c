// expect: rejected cerastes-unsupported_annotation @ L10
// twin: tests/memory/not_invalidating_int_pointer.c
// c-only: a redeclaration of a system function; glibc's C++ declarations carry an exception specification that conflicts; `time`, as reject_marker_system_redeclaration.c
// Pins: 0058 item 2 ("Never ignored"): not_invalidating on a user redeclaration of a function first declared in a system header
// Pre-registered from record 0058 item 2: rejected unsupported_annotation at the declaration
#include <cerastes.h>

#include <time.h>

time_t time(time_t *timer CERASTES_NOT_INVALIDATING);  // <- here

int main(void)
{
  return 0;
}
