// expect: verified
// expect: native-ok
// mode: c23, c++20
// (a comment line keeps the line numbers of the program this test replaces)
// Pins: 0064 items 1 and 9 (Flips A of Part C2, the staging pin negative/global_array_reject_struct_elements.c retired): a global array of structures with external linkage and no initializer is a declared one-block global, and a static local array of structures is read in a function with no contract; the program is unchanged, line for line (twin negative/global_array_struct_unknown_check.c)
// Pre-registered from record 0064 items 1 and 9 (Part C2), before any run: verifies
// (a second comment line keeps the line numbers)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>
struct P {
    int x;
    int y;
};
struct P g[3];
int f_c08(void)
{
    return 0;
}

int f_c10(void)
{
    static struct P t[2];
    return t[0].x;
}
