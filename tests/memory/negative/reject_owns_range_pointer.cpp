// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c++20
// twin: tests/memory/owns_inc.c
// Pins: 0060 item 3 (0040 item 2; flip B): owns_range of a const pointee is rejected, and since M4.5e1b the rejection is the pointee's, not the range atom's: "cerastes::owns_range" of a const place (a function that only reads p[0..n) takes reads_range). tests/memory/range_cpp_forms.cpp's first_deref is the positive of this program with reads_range
#include <cerastes.h>
#include <stddef.h>

int first(const int *p)
    CERASTES_REQUIRES(cerastes::owns_range(p, 2))
{
  return *p;
}
