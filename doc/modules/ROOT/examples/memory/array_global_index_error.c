// expect: error out-of-bounds @ L12
// mode: c23, c++20
// The index may be one past the array: the access is outside its four elements.
// tag::source[]
#include <cerastes.h>

static int table[4] = {10, 20};

void store(int i, int v)
    CERASTES_REQUIRES(cerastes_owns(table) && 0 <= i && i <= 4)
{
  table[i] = v;
}

int main(void)
{
  return table[0] - 10;
}
// end::source[]
