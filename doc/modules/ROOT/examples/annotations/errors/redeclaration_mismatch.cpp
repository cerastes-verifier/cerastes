// expect: frontend-error attr_redeclaration_mismatch @ L7
// Two declarations that carry different contracts.
// tag::bad[]
#include <cerastes.h>

int bump(int x) CERASTES_REQUIRES(x > 0);
int bump(int x) CERASTES_REQUIRES(x > 10); // <- error
// end::bad[]
