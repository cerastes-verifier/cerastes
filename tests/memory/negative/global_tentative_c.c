// expect: error check @ L11
// c-only: -fcommon merges a tentative definition in C; in C++ the value of an external-linkage global is unknown anyway (global_linkage_cpp_unknown.cpp)
// twin: tests/memory/global_linkage_c.c
// Pins: 0041 item 3 (audit S1): a tentative definition with external linkage may merge with another unit's definition (-fcommon, Apple clang's default): its value is unknown in main, so g == 0 is not a fact
#include <cerastes.h>

int g;

int main(void)
{
  CERASTES_CHECK(g == 0);
  return 0;
}
