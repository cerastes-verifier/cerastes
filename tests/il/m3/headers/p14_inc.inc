// A user header for probe P14: it has a function, then a #line directive and a second function.
inline int from_header(int a) { return a + 1; }
#line 500 "renamed_in_header.h"
inline int after_line_in_header(int a) { return a + 2; }
