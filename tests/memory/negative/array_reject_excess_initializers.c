// expect: rejected excess_initializers_ignored @ L11
// mode: c23
// c-only: in C++ the same list is a front-end error (the _cxx twin)
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 3 (P78 update): in C the IL drops the extra elements, so the driver turns EDG's warning #1162 (excess initializers are ignored) into an error in verify mode; the pin is EDG's tag, as for a front-end error (in C++ the list is EDG's own error #146: the _cxx twin)
// Pre-registered from the P78 update of record 0059 item 3: no cerastes tag, EDG's #1162 as an error
#include <cerastes.h>

int f(void)
{
  [[maybe_unused]] int a[3] = {1, 2, 3, 4};
  return 0;
}
