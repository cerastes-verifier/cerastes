// expect: rejected wrong_entity_for_attribute @ L7
// mode: c23, c++20
// twin: tests/memory/init_ref_to_uninit.c
// Pins: 0042 item 2 (P54, P64): ref_to_uninit on a function is EDG's own refusal, re-expected by M4.5d2 (target r)
#include <cerastes.h>

CERASTES_REF_TO_UNINIT int *get(void);  // <- here
