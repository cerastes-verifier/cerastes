// expect: error out-of-bounds @ L13
// mode: c23, c++20
// twin: tests/memory/global_array_values.c
// Pins: 0063 item 3: the array global has three elements: the read at the variable index 3 is out-of-bounds (the index is a variable, so the front end's constant-index error, subscript_out_of_range, does not apply)
// Pre-registered from record 0063 item 3 (Part B), before any run: the one error
#include <cerastes.h>

static int full[3] = {1, 2, 3};

int main(void)
{
  int i = 3;
  return full[i];  // <- here
}
