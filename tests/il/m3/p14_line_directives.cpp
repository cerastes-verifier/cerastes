// expect: frontend-ok
// mode: c++20
// M3 probe P14: #line directives and GNU linemarkers in the primary file and in a user header.
#include "headers/p14_inc.inc"
int plain_before(int a) { return a; }
#line 100 "presumed.h"
int after_line(int a) { return a + 3; }
# 12 "marker.h" 3
int after_system_marker(int a) { return a + 4; }
# 40 "tests/il/m3/p14_line_directives.cpp" 1
int after_return_marker(int a) { return a + 5; }
#line 70
int after_number_only(int a) { return a + 6; }
