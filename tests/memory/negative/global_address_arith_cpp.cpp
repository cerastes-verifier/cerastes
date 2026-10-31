// expect: rejected cerastes-unsupported_expression @ L12
// mode: c++20
// twin: tests/memory/global_unnamed_namespace.cpp
// Pins: 0041 item 6 (audit B2): the front end folds &g + 1 == &h to false, though a one-past-the-end pointer may equal the next object: the original expression is encoded, and its pointer arithmetic is rejected until M4.5e2
#include <cerastes.h>

static int g = 0;
static int h = 0;

int main()
{
  if (&g + 1 == &h) {
    return 1;
  }
  return 0;
}
