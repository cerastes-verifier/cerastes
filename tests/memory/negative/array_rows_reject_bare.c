// expect: rejected cerastes-unsupported_expression @ L15
// expect: rejected cerastes-unsupported_expression @ L20
// expect: rejected cerastes-unsupported_expression @ L28
// expect: rejected cerastes-unsupported_expression @ L34
// expect: rejected cerastes-unsupported_expression @ L39
// expect: rejected cerastes-unsupported_expression @ L45
// mode: c23, c++20
// twin: tests/memory/array_rows_param.c
// Pins: 0064 item 13 (the coordinator's ruling of 2026-11-26): a bare read of a row parameter (a == b, if (a), a = b, (void)a), *a and (*a)[1] is rejected by the encoder where it is read: the parameter is read only as the base of an index or as a whole argument
// Pre-registered from record 0064 item 13 (Part E), before any run: rejected (tag unsupported_expression by analogy; not stated by the record: to confirm at the first run, question 160)
#include <cerastes.h>

int equal(int a[][3], int b[][3])
{
  return a == b;  // <- here
}

int truth(int a[][3])
{
  if (a) {  // <- here
    return 1;
  }
  return 0;
}

void assign(int a[][3], int b[][3])
{
  a = b;  // <- here
  a[0][0] = 0;
}

void discard(int a[][3])
{
  (void)a;  // <- here
}

int deref(int a[][3])
{
  int* p = *a;  // <- here
  return p[0];
}

int deref_row(int a[][3])
{
  return (*a)[1];  // <- here
}
