// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Running Silicon as a child process (decision 0011 item 11).  No EDG headers.
#pragma once

#include <string>

namespace cerastes {

struct SiliconTools
{
    std::string java; // the JDK 17 `java`
    std::string jar;  // silicon.jar
    std::string z3;   // the pinned Z3
};

struct SiliconRun
{
    bool started {false};   // the process was created
    bool timed_out {false}; // killed at the deadline
    bool signaled {false};  // ended by a signal (not ours)
    int exit_code {-1};     // the exit status when it ended normally
    std::string output;     // standard output and standard error, in the order written
    std::string error;      // why `started` is false, or other trouble, "" for none
    // The name of the .vpr file given to Silicon: Silicon prints it in its error positions.
    std::string vpr_name;
};

// The number of provers Silicon runs in parallel (`--numberOfParallelVerifiers`): the hardware's thread count, at
// least 1 (`hardware_concurrency` may be 0) and at most 16.  Silicon's own default is one per core, which costs about
// 33 s of start-up on a 172-core machine (owner question 286); on a machine of 16 threads or fewer the cap changes
// nothing.
// scripts/run_tests.py's run_silicon uses the same rule.
int silicon_prover_count(unsigned hardware_threads);

// Writes `vpr` as program.vpr in a new private temporary directory, runs
//     java -Xss128m -jar <jar> --z3Exe <z3> --numberOfErrorsToReport 0 --numberOfParallelVerifiers <n> program.vpr
// (n = silicon_prover_count(hardware threads): at most 16, decision 0011 item 11)
// there with a clean environment, and removes the directory.  `timeout_seconds` is a wall-clock limit for the
// whole process group (0 kills it at once); it is killed, with Z3, when the limit passes.
SiliconRun run_silicon(const SiliconTools& tools, const std::string& vpr, int timeout_seconds);

// Async-signal-safe cleanup for a SIGINT or SIGTERM that arrives while Silicon runs (decision 0011 item 11): kills
// Silicon's process group (with Z3) and removes its temporary files.  The caller then exits.  Does nothing when no
// Silicon is running.
void silicon_signal_cleanup();

} // namespace cerastes
