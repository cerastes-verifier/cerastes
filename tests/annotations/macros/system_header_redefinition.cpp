// expect: golden
// expect: native-ok
// twin-exempt: the must-fail cases report in a header, which no expect line can name; they are SystemHeaderMacroTests in scripts/test_run_tests.py
// Pins: decision 0008 item 2 as amended by record 0031 item 9: in a system file, an #undef of a predefined macro followed by its identical #define changes nothing (glibc's <stdint.h> and __INT64_C)
#include "headers/predefined_again.h"
#include <cerastes.h>

int half(int x) CERASTES_REQUIRES(x >= 0);
