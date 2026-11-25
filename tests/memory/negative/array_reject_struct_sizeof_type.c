// expect: rejected cerastes-unsupported_type @ L15
// mode: c23, c++20
// twin: tests/memory/array_sum_two.c
// Pins: M4.5e1e2 Part E retires or re-expects this staging pin (the orchestrator, from the plan's part table: record item 13's computed types are Part E's; `sizeof(v)` of a named local array of structures is Part B's and is not this file) (plan section 10, Flips A; R2 of Step 0): the type operand sizeof(struct P[3]), an array of structures as a type. The rows are what the frozen base build (f07d80a) printed in both modes
// Pre-registered from the base build's R2 run, before any part of M4.5e1e2: rejected (observed, not predicted)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>
struct P {
    int x;
    int y;
};
int f_b21(void)
{
    return (int)sizeof(struct P[3]); // <- here
}
