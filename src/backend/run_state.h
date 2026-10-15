// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// What the driver needs to know about a run: did back_end() run, and is the
// front end in a mode cerastes supports (decisions 0006, 0007, 0008).
#pragma once

namespace cerastes {

// Called first thing by back_end().
void note_back_end_ran();
bool back_end_ran();

// Refuses, with one `cerastes-fe: error: ...` line on stderr and exit status 2,
// a language mode that cerastes does not support: C before C23, and GNU
// emulation.  The driver refuses the options that select them by name; this is
// the check on the state itself, for spellings the driver does not see (an
// abbreviation it resolves differently, an options file, the default C mode).
void refuse_unsupported_mode();

// Registered with atexit() by the driver: if the front end finished without
// errors but back_end() never ran (--no_code_gen, or an abbreviation of it),
// nothing was checked, so say so and exit with status 2.
void guard_back_end_ran();

// The refusal line: `cerastes-fe: error: <message>`.
void print_refusal(const char* message);

}  // namespace cerastes
