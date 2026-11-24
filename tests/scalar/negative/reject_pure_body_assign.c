// expect: rejected cerastes-annotation_side_effect @ L13
// expect: frontend-warning set_but_not_used @ L10
// mode: c23, c++20
// twin: tests/scalar/pure_check_small.c
// Pins: 0062 item 3: an assignment in the body of a read pure function is annotation_side_effect (the tag it has in an annotation, forall_reject_e_assign.c)
// Pre-registered from record 0062 (Part B), before the code: rejected (EDG's own warning on the assigned parameter, measured on the frozen base build, is listed). the line is a question-160 candidate (the construct's own line: the function is read by its requires alone)
#include <cerastes.h>


CERASTES_PURE bool g(int x)
    CERASTES_REQUIRES(0 <= x)
{
  return (x = 0) == 0;
}

int use(int y)
{
  return y;
}
