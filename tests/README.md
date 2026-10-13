# Tests

Run everything with `python3 scripts/run_tests.py` (or a list of files and
directories). `scripts/check.sh` runs it after the harness self-test
(`scripts/test_run_tests.py`). `-v` prints full diffs and tool output for
failing tests. The exit status is 0 when every selected test passes, 1 when
any fails and 2 on usage or environment errors, including when nothing is
left to run.

The harness self-test (`scripts/test_run_tests.py`) has end-to-end classes
that need the built front end. They are skipped when it is missing, unless
`CERASTES_REQUIRE_E2E=1` (which `scripts/check.sh` sets), in which case they
fail, so a missing build cannot silently switch them off.

| Directory | What it checks | Runner |
|---|---|---|
| `viper-smoke/` | Silicon and Z3 work, and the harness reads Silicon's errors correctly | Silicon |
| `edg-smoke/` | The pinned EDG front end parses C++20 and C23 against the system headers | `scripts/cpfe.sh` |
| `il/` | Golden tests of the IL walker (`--cerastes_walk`): the Gate G1 probes (`g1/`), annotation-free copies of the VeriFast tests (`verifast-plain/`), contrast probes that expose a vacuous pass (`contrast/`) and walker smoke tests | `scripts/cerastes-fe.sh` |
| `differential/verifast/` | Nothing yet: unmodified reference copies of VeriFast's template tests, with no `expect:` headers. Skipped (`.harness-skip`) | none |

Later milestones add `annotations/`, `scalar/`, `templates/`, `concepts/`
and `memory/`, and the differential tests against VeriFast in M4 (see
plan.md).

## Header format

Every test file starts with a block of `//` comment lines. The `expect:` and
`mode:` lines in that block are the test's expectations; other comment lines
describe the test. An `expect:` or `mode:` line after the first line of code
is an error, so an expectation can never be silently ignored.

```
// expect: verified
// expect: error <category>[:<reason>] @ L<line>
// expect: rejected <tag>
// expect: frontend-ok
// expect: golden
// mode: c++20 | c23
```

- **`verified`**: verification succeeds with no errors. It must be the only
  `expect:` line.
- **`error <category>[:<reason>] @ L<line>`**: one line per expected error.
  The test passes only if the reported errors are exactly these: each one
  with that category, at that line, and with that reason if one is given,
  and no others. The harness says whether a miss was the *wrong line*, the
  *wrong reason*, a *missing* error or an *unexpected* one.
- **`rejected <tag>`**: the driver refuses the program with that diagnostic
  tag (for example `unsupported-union`). Needs the driver (M3).
- **`frontend-ok`**: the EDG front end accepts the file with exit status 0
  and prints nothing. For front-end smoke tests only.
- **`golden`**: the IL walker's output equals the test's golden file, byte
  for byte. It must be the only `expect:` line, and it applies to `.c` and
  `.cpp` files only. See [Golden tests](#golden-tests).
- **`mode:`**: the language modes to run a C or C++ test in, comma-separated
  or on several lines. Each mode is a separate test run. The default is
  `c++20` for `.cpp` and `c23` for `.c`. Not allowed in `.vpr` files.

Line numbers are 1-based lines of the test file itself. Solver timeouts,
crashes, parse or type errors and messages the harness cannot classify always
fail the test, whatever the expectation.

**Must-fail twin rule:** every must-verify test that exercises an encoding
rule has at least one twin that must fail for the right reason at the right
line.

## Golden tests

For each mode of a `golden` test the harness runs, from the repository root,

```
scripts/cerastes-fe.sh <mode option> --cerastes_walk <test path relative to the root>
```

(`--c++20` or `--c23`; it needs `CERASTES_FE`, which `scripts/bootstrap.sh`
writes into `build/env.sh`). The output it compares is the front end's
stdout, then, only if stderr is not empty, a line `--- stderr ---` followed by
stderr (if stdout then lacks a final newline, a line
`\ No newline at end of stdout` comes first, so the difference stays
visible). The two streams are never interleaved. Running from the root keeps
the paths in diagnostics stable.

The expected output is in a golden file next to the test:

- `<test>.golden` when the test has one mode (the default mode or one
  `mode:` line), for example `il/g1/check1_inc.cpp.golden`;
- `<test>.<mode>.golden` for each mode when it has several, for example
  `x.c.c++20.golden` and `x.c.c23.golden`.

The walker's stdout must end with the line

```
end walk: <R> routines reported, <U> routines with bodies in non-system files outside the primary file
```

(`R` and `U` are numbers). It is the last line of stdout, before any
`--- stderr ---` section, and it proves that the walk ran to the end. A run
without it (a crash, a run that printed only diagnostics, a line that is
missing, malformed or followed by more output) fails with "walker did not
finish" even when its output equals the golden file, and `--update-golden`
refuses to write it.

The test fails if the golden file is missing (it is never created by a normal
run), if the front end exits with a nonzero status, crashes or times out
(even when its output matches), if it prints nothing at all (an empty golden
would pass vacuously), or on any difference, including whitespace at line
ends, line endings and the final newline. A difference is shown as a unified
diff from the golden file (`-`) to the actual output (`+`), with whitespace at
line ends shown as `<SP>`, `<TAB>` or `<CR>`; long diffs are cut short unless
`-v` is given.

Golden files are not tests themselves (`.golden` is not a test suffix). A
golden file that no golden test uses (left behind by a renamed test, a
changed mode list or a changed expectation) is an *orphan*: both
`--check-headers` and normal runs report it as an error.

**Writing golden files.** `python3 scripts/run_tests.py --update-golden
<files or dirs>` writes the golden files of the selected golden tests from the
current output instead of comparing, and prints `UPDATED <golden file>` with
the diff (or `UNCHANGED <golden file>`). It never writes the output of a run
that failed or printed nothing. A golden file is an expectation, so
invariant 1 applies to it: regenerate it only for an intended change, review
the diff line by line, and state the reason in the commit message. Never
regenerate a golden file to make a test pass. `scripts/check.sh` never uses
`--update-golden`, and the harness self-test fails if a script does.

## Files that are not tests

Under a scanned directory, a file whose suffix says C, C++ or Viper source
but is not a test suffix (`.vpr`, `.c`, `.cpp`) would silently never run, so
it is an error, in normal runs and with `--check-headers`: `.cc`, `.cxx`,
`.c++`, `.cp`, `.CPP`, `.C`, `.h`, `.hh`, `.hpp`, `.hxx`, `.ipp`, `.inl`, `.tcc`,
`.i`, `.ii` and `.sil` (the comparison ignores case, except that the exact
`.c`, `.cpp` and `.vpr` are tests). Rename such a file or remove it; a test
cannot keep a helper header next to it. Skipped directories are exempt (see
below), and other files (`.md`, `.golden`, ...) are ignored.

## Skipped directories

A directory that contains a file named `.harness-skip` is skipped, with all
its subdirectories, by test runs and by `--check-headers`, even when a file
inside it is named on the command line. The marker holds a one-line reason;
an empty or multi-line marker is an error.

A marker is permitted only in the directories listed in `SKIP_ALLOWED` in
`scripts/run_tests.py`, which is currently `tests/differential/verifast`
alone. A marker anywhere else, including `tests/il`, is an error (exit 2) that
names the marker file, so that nobody can turn a directory of real tests green
by adding one. Adding a directory to the list is a reviewed change; the
harness self-test checks that the real tree has no other marker.

A file in a skipped directory that starts with an `// expect:` header is a
real test that would never run; that is an error too, in normal runs and with
`--check-headers`.

The harness prints `SKIP  <directory>: <reason>` once per skipped directory and counts skipped
directories separately (`N passed, M failed, K skipped`); a skipped directory
never counts as passed. Use it only for files that are not tests, such as
the reference copies in `differential/verifast/`.

## Categories for `.vpr` tests

Silicon prints only a readable message per error. `scripts/run_tests.py`
recovers the Viper error kind from the fixed text each kind starts with (from
silver's `VerificationError.scala` at the pinned commit), and
`viper-smoke/fail_kinds.vpr` checks that table against the pinned Silicon.

| Category | Viper error id | Message starts with |
|---|---|---|
| `assert` | `assert.failed` | Assert might fail. |
| `postcondition` | `postcondition.violated` | Postcondition of M might not hold. |
| `precondition` | `call.precondition`, `application.precondition` | The precondition of method M… / Precondition of function F… |
| `invariant-entry` | `invariant.not.established` | Loop invariant I might not hold on entry. |
| `invariant-preserved` | `invariant.not.preserved` | Loop invariant I might not be preserved. |
| `assignment` | `assignment.failed` | Assignment might fail. |
| `call` | `call.failed` | Method call might fail. |
| `exhale` / `inhale` | `exhale.failed` / `inhale.failed` | Exhale/Inhale might fail. |
| `fold` / `unfold` | `fold.failed` / `unfold.failed` | Folding/Unfolding P might fail. |
| `if` / `while` | `if.failed` / `while.failed` | Conditional/While statement might fail. |
| `wellformedness` | `not.wellformed`, `function.not.wellformed`, `predicate.not.wellformed`, `wand.not.wellformed` | … might not be well-formed. |
| `termination` | `termination.failed` | Function F might not terminate. |
| `package` / `apply` / `letwand` | `package.failed` / `apply.failed` / `letwand.failed` | Packaging/Applying/Referencing a wand… |
| `quasihavoc` | `quasihavoc.failed`, `quasihavocall.failed` | Quasihavoc(all) might fail. |
| `heuristics` / `internal` | `heuristics.failed` / `internal` | Applying heuristics failed. / An internal error occurred. |

Reasons are Viper reason ids: `assertion.false`, `insufficient.permission`,
`division.by.zero`, `receiver.null`, `negative.permission`,
`permission.not.positive`, `qp.not.injective`, `seq.index.negative`,
`seq.index.length`, `map.key.contains`, `labelled.state.not.reached`,
`wand.not.found`, `invalid.perm.multiplication`, `epsilon.as.param`,
`quasihavocall.not.injective`, `feature.unsupported`, `unexpected.node`.

Categories for C and C++ tests are defined by the driver's error messages
(M3) and will be added here then.
