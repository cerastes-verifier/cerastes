// expect: rejected cerastes-unsupported_annotation @ L9
// twin: tests/memory/init_must_init.c
// c-only: a redeclaration of a system function; glibc's C++ declarations carry an exception specification that conflicts
// Pins: 0042 item 4 (audit S5): a marker on a user redeclaration of a function first declared in a system header is never silently ignored
#include <cerastes.h>

#include <stdlib.h>

long strtol(const char *nptr, char **endptr CERASTES_MUST_INIT, int base);  // <- here

int main(void)
{
  return 0;
}
