// expect: rejected cerastes-unsupported_annotation @ L7
// mode: c23, c++20
// twin: tests/memory/init_ref_to_uninit.c
// Pins: 0047 item 5 (0042 item 2; flipped by M4.5d2a, approved with its plan): ref_to_uninit on a function is registered (target r), and the marker says the block the function hands over holds no value, so it needs `owns_block(r)` in an `ensures`; without one it is unsupported_annotation
#include <cerastes.h>

CERASTES_REF_TO_UNINIT int *get(void);  // <- here
