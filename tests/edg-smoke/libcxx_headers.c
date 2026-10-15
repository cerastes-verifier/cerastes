// expect: frontend-ok
// mode: c23
//
// Smoke test: the pinned EDG front end parses the C library headers of the
// system SDK that include <stdlib.h> and <string.h> in C23 mode (stock cpfe,
// native macOS configuration).  <stdlib.h> uses blocks (qsort_b, bsearch_b and
// others) and failed before EDG bug 0005 (internal/edg_bugs) was fixed.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
  char buf[8];
  strcpy(buf, "hi");
  return atoi("0") + (int)strlen(buf) - 2;
}
