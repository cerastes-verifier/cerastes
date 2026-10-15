// expect: frontend-error cerastes-annotation_side_effect @ L9
// expect: native-ok
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: an annotation in a file the front end treats as a system header (a `# N "file" 3` line marker in the primary file keeps the file name and the line numbers): the checks must still run there.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

# 9 "tests/annotations/negative/sys_file_annotation_side_effect.cpp" 3
int half(int x) CERASTES_REQUIRES(x++ >= 0);
