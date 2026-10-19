// expect: frontend-ok
// mode: c23
// M3 probe P14 (C23): #line directives and GNU linemarkers in the primary file and in a user header.
#include "headers/p14_inc.inc"
int plain_before(int a) { return a; }
#line 100 "presumed.h"
int after_line(int a) { return a + 3; }
# 12 "marker.h" 3
int after_system_marker(int a) { return a + 4; }
#line 70
int after_number_only(int a) { return a + 6; }
