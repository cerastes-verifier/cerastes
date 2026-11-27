// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// twin: tests/memory/array_rows_param.c
// Pins: 0064 item 13 ("Markers on a row parameter are not built", M4.5e2): cerastes::owning(n) on a row parameter int a[][3] is rejected at the function's head, where a marker over rows would need the closure of an owning count over two index steps
// Pre-registered from record 0064 item 13 (Part E), before any run: rejected, unsupported_annotation at the function's head
#include <cerastes.h>

void f(int n, int a CERASTES_OWNING_N(n) [][3])  // <- here
{
  a[0][0] = n;
}
