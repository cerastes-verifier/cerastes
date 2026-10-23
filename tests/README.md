# Tests

Run everything with `python3 scripts/run_tests.py` (or a list of files and
directories). With no arguments it runs the default paths, `tests/` and the
user-documentation examples in `doc/modules/ROOT/examples/` (see
[Documentation examples](#documentation-examples)); paths named on the command
line replace them. A default directory that does not exist is an error (exit
2) that names it, so a deleted examples directory cannot silently drop the doc
tests. `scripts/check.sh` runs the harness after its self-test
(`scripts/test_run_tests.py`). `-v` prints full diffs and tool output for
failing tests. The exit status is 0 when every selected test passes, 1 when
any fails and 2 on usage or environment errors, including when nothing is
left to run.

The harness self-test (`scripts/test_run_tests.py`) has end-to-end classes
that need the built front end. They are skipped when it is missing, unless
`CERASTES_REQUIRE_E2E=1` (which `scripts/check.sh` sets), in which case they
fail, so a missing build cannot silently switch them off.

The verify tests (`verified`, `error`, `rejected` in C and C++) are tested against a
fake `cerastes verify` that the self-test writes, so those checks always run; a
class that runs the real driver is added with the driver (M3-3).

| Directory | What it checks | Runner |
|---|---|---|
| `viper-smoke/` | Silicon and Z3 work, and the harness reads Silicon's errors correctly | Silicon |
| `edg-smoke/` | The pinned EDG front end parses C++20 and C23 against the system headers | `scripts/cpfe.sh` |
| `il/` | Golden tests of the IL walker (`--cerastes_walk`): the Gate G1 probes (`g1/`), annotation-free copies of the VeriFast tests (`verifast-plain/`), contrast probes that expose a vacuous pass (`contrast/`) and walker smoke tests | `scripts/cerastes-fe.sh` |
| `scalar/` | M3: scalar code through `cerastes verify` (`verified` positives, each with a must-fail twin in `scalar/negative/`) | `build/cerastes verify` |
| `differential/verifast/` | Nothing yet: unmodified reference copies of VeriFast's template tests, with no `expect:` headers. Skipped (`.harness-skip`) | none |
| `../doc/modules/ROOT/examples/` | The code examples of the user documentation, which are ordinary tests (not under `tests/`, but run by default with it) | by the example's `expect:` kind |

`annotations/` and `scalar/` each have a `negative/` directory of must-fail twins
(see [Twins](#twins)). Later milestones add `templates/`, `concepts/` and
`memory/`, and the differential tests against VeriFast in M5 (see plan.md).

## Header format

Every test file starts with a block of `//` comment lines. The `expect:` and
`mode:` lines in that block are the test's expectations; other comment lines
describe the test. An `expect:`, `mode:` or `twin:` line after the first
line of code is an error, so an expectation can never be silently ignored.

```
// expect: verified
// expect: error <category> @ L<line>
// expect: error <category>[:<reason>] @ L<line>      (.vpr files: Viper reasons)
// expect: rejected <diag> @ L<line>      (may be followed by frontend-warning lines: its expected warnings)
// expect: frontend-ok
// expect: frontend-error <diag> @ L<line>
// expect: frontend-warning <diag> @ L<line>
// expect: frontend-refused <substring>
// cerastes-fe-options: <options>
// expect: golden
// expect: native-ok
// mode: c++20 | c23
// twin: <repo-relative path of a test>
```

- **`verified`**: verification succeeds with no errors (`.vpr` through Silicon,
  `.c`/`.cpp` through `cerastes verify`). It must be the only outcome line
  (`native-ok` may accompany it).
- **`error <category> @ L<line>`**: one line per expected error.
  The test passes only if the reported errors are exactly these: each one
  with that category, at that line, and no others. The harness says whether
  a miss was the *wrong line*, the *wrong reason* (a wrong category), a
  *missing* error or an *unexpected* one. In `.vpr` tests the category is a
  Viper error kind and `:<reason>` (a Viper reason) may follow it; in `.c` and
  `.cpp` tests the category is one of the driver's ([Categories](#categories-for-c-and-c-tests))
  and a `:<reason>` is a header error.
- **`rejected <diag> @ L<line>`**: one line per diagnostic. `cerastes verify`
  does not accept the program (exit status 2) and prints exactly these errors.
  `<diag>` is an EDG tag or `cerastes-<tag>`, as for `frontend-error`
  (validated the same way). `.c` and `.cpp` only. Not mixed with `error`.
  See [Verify tests](#verify-tests-verified-error-rejected).
- **`frontend-ok`**: the EDG front end accepts the file with exit status 0
  and prints nothing. For front-end smoke tests only.
- **`frontend-error <diag> @ L<line>`** and **`frontend-warning <diag> @ L<line>`**:
  the front end prints exactly these errors and warnings, each with that
  diagnostic at that line. `<diag>` is an EDG diagnostic tag or
  `cerastes-<tag>`. Any number of lines; not mixed with the other outcome
  kinds; `.c` and `.cpp` only. See [Front-end diagnostics](#front-end-diagnostics).
- **`frontend-refused <substring>`** with **`cerastes-fe-options: <options>`**:
  the driver refuses these command-line options. See
  [Refused options](#refused-options).
- **`golden`**: the IL walker's output equals the test's golden file, byte
  for byte. It must be the only outcome line (`native-ok` may accompany it),
  and it applies to `.c` and `.cpp` files only. See [Golden tests](#golden-tests).
- **`native-ok`**: Apple clang, and GCC when installed, also accept the file
  with `-Wall -Wextra -Werror` and without `__CERASTES__`. It accompanies an
  outcome line and is a header error alone, twice or in a `.vpr` file. See
  [Ordinary compilers](#ordinary-compilers-native-ok).
- **`twin: <path>`**: the positive test a must-fail test was derived from.
  See [Twins](#twins).
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

## Front-end diagnostics

`frontend-error` and `frontend-warning` tests check what the EDG front end,
with the cerastes back end behind it, says about a file. For each mode the
harness runs, from the repository root,

```
scripts/cerastes-fe.sh <mode option> --brief_diagnostics --display_error_number --no_colors <test path relative to the root>
```

(never `--cerastes_walk`, never `--no_code_gen`: the back end has to run, or its
own diagnostics could never appear). The test passes only if stdout is empty and
the diagnostics on stderr are exactly the expected ones.

A `frontend-warning` test with one deliberate warning (for example
`[[unreg_ns::x]] int g;`, an unrecognized attribute namespace), in a file of
includes and code under test, is the way to run cerastes-fe to the end on code
that has no golden: the test passes only if that warning is the
only diagnostic, so any error in the includes or the code, or a front end that
stopped early, fails it (see `edg-smoke/libcxx_headers_cerastes_fe.cpp`).

**The diagnostic names.** `<diag>` is the tag of EDG's diagnostic, the name in
`error_msg.txt` without its `ec_` prefix (`undefined_identifier`,
`attribute_namespace_unrecognized`), not its number. Tags are validated, so a
misspelling is a header error with a suggestion, not a test that fails
mysteriously. The tables are EDG's generated `err_data.h` (the tag table, lines
`{ "tag", ec_name }`, `NUMBER_OF_ERROR_TAGS` of them) and `err_codes.h` (the
number of each `ec_name /* = N */`) in `$CERASTES_EDG_GEN`, the stock build of the
*active* EDG tree (decision 0005): they are generated from that tree's own
`error_msg.txt`, so a fork that adds diagnostics brings its own tags. The
harness refuses tables that it cannot read completely (a declared count that
differs from the entries parsed, a tag whose code has no number, a repeated
tag). `cerastes-<tag>` names a diagnostic of the cerastes back end, which prints
`#cerastes-<tag>` where EDG prints the number; it is validated against
`src/backend/diagnostics.def` (lines `MACRO(tag, ...)`). Until that file exists,
a `cerastes-` tag is a header error ("no cerastes diagnostics are defined
yet").

**What is compared.** Primary diagnostic lines start in column 0:

```
"file", line N (col. C): error #NNNN[-D]: message
"file", line N (col. C): warning #NNNN-D: message
```

(`-D` marks a discretionary diagnostic and is ignored.) `note` and `remark`
lines, and the indented lines that continue a diagnostic (argument lists,
instantiation contexts), belong to the previous diagnostic and do not count.
The harness compares the multisets of (tag, line) for errors and for warnings
separately, so a repeated line expects a repeated diagnostic. It says whether a
miss was the *wrong line* (same tag elsewhere), a *wrong diagnostic at this
line*, a *missing* one or an *unexpected* one, with the message text and column
of what was printed. A diagnostic in another file (a header the test includes)
is unexpected, whatever it is.

**Always a failure**, whatever the test expects: a catastrophic, command-line or
internal error, `Compilation terminated.`/`aborted.`, `Error limit reached.`, a
crash, a timeout, anything on stdout, and any other line starting in column 0 (a
format the parser does not know, reported as "unrecognized front-end output").
The `N errors detected in the compilation of ...` summary must agree with the
number of errors parsed (and is required when there are errors). The exit
status must be 2 when at least one error is expected and 0 otherwise.

**Limitation.** EDG's errors stop the back end, so a test cannot expect an EDG
error and a back-end diagnostic together. Write two tests.

`--update-golden` leaves these tests alone. They need `CERASTES_FE`,
`CERASTES_EDG_GEN` and `EDG_CLANG_VER_SCRAPE` from `build/env.sh`.

## Verify tests (`verified`, `error`, `rejected`)

A `.c` or `.cpp` test with one of these outcomes runs the verifier (decision 0011).
For each mode of the test the harness runs, from the repository root,

```
build/cerastes verify <test path relative to the root> --std=<c++20|c23> --keep-vpr=build/vpr/<test path>.<mode>.vpr
```

(the program is `CERASTES_VERIFY` of `build/env.sh`, which `scripts/bootstrap.sh`
writes from M3 on; the harness needs it, and treats these tests like the other
tests that run a tool built from the EDG tree, see
[The EDG tree](#the-edg-tree-decision-0005)). One run per mode; a test passes
only if every mode passes. The exit status says what happened:

| Exit | Meaning | What the harness reads |
|---|---|---|
| 0 | verified | on stdout exactly one line `<test path>: verified (<n> functions; one translation unit: ...)` with `n >= 1`, no `error:` line, nothing on stderr |
| 1 | verification errors | on stdout one or more lines `<file>:<line>: error: <category>: <message>`, each possibly followed by `<file>:<line>: note: ...` lines, sorted by printed path (byte order), line and category (byte order) and none twice; nothing on stderr |
| 2 | the program is not accepted (a front-end error, a cerastes rejection) | EDG-format diagnostics on stderr, as in [Front-end diagnostics](#front-end-diagnostics) (`"file", line N: error #<id>: ...`, with the `N errors detected` summary), nothing on stdout |
| 3 | tool failure (Silicon crashed or timed out, an error the position map does not explain) | always a failing test, whatever the test expects |

A timeout, a crash (a signal) and any other exit status always fail too. The
streams are those of decision 0011 item 9: the verdict, the errors and their notes
are on **stdout**, front-end diagnostics on **stderr**. For exit status 0 and 1
a verdict or an error on stderr fails the test, and so does anything else on
stderr (a front-end warning in a test is unexpected output: tests are
warning-free) and a front-end diagnostic on stdout. A blank line or an indented
line continues the previous line and is ignored, and a `note:` line is ignored but
must follow an error. Any other line in column 0 of stdout (a note with no error,
a line the harness does not know) fails the test as unrecognized driver output.
Errors out of order, or printed twice, fail it too.

**Where an error is.** `@ L<n>` is a line of the test file. An error in a user
header (decision 0011 item 4 verifies them) is expected with
`// expect: error <category> @ <repository-relative file>:L<n>`, and a rejection
the same way (`rejected <diag> @ <file>:L<n>`); the file is compared with what the
driver prints. An error in a file that no expectation names is a failure. The path
has no `..`, `.` or empty component, and `@ <file>:L<n>` is not for `.vpr` or
`frontend-*` lines.

**`verified`** passes with exit status 0. **`error`** passes with exit status 1
and exactly the expected errors: the multiset of (category, line) is compared as
for `.vpr` tests (*wrong line*, *wrong reason*, *missing*, *unexpected*); an
error reported in another file (a header) is a failure, so is a category the
driver does not define (`unknown`, "unrecognized error"). **`rejected`** passes
with exit status 2 and exactly the expected errors as (diagnostic, line), with
the comparison, the fatal diagnostics (catastrophic, command-line, internal) and
the summary check of `frontend-error` tests; warnings are not expected, so a
warning fails, unless the test lists it: a `rejected` test may also carry
`// expect: frontend-warning <diag> @ L<n>` lines (one per warning), and then the multiset of
(warning tag, line) the front end prints must equal them exactly (a listed warning that is
not printed fails as *missing*, an unlisted one as *unexpected*; the same tag at another line is
*wrong line*). Without such lines no warning is allowed. A `frontend-error` line does not mix
with `rejected`, and `frontend-warning` lines without a `rejected` line are a front-end test.
The cerastes tests use it for EDG's #174 (`expr_has_no_effect`), which the owner decided
not to suppress: it fires on every comma operator whose left operand has no effect, which
`comma_without_effect` rejects. A verdict that is not the expected one is named: a `verified` test
that is rejected, an `error` test that verified, a `rejected` test that reports
verification errors.

**Viper output.** For a `verified` or `error` test (a run that ended with exit
status 0 or 1) the driver must leave the file given by `--keep-vpr=<path>` and
its position map `<path>.map` (decision 0011 item 2); a missing or empty file
fails the test. The position map (tab-separated, the file the 4th field:
encoding.md section 5) must have at least one entry whose file is the test, so a
test none of whose bodies was verified (all skipped, or taken for system code)
cannot pass. The harness deletes both files before each run. A run that selects
such a test first clears `build/vpr/` and writes `build/vpr/MANIFEST`: a first line
`# run-id: <UTC time>-<hash of the file list>` (the run prints it as
`build/vpr: run <id>, <n> files expected`), then one line per file it expects
(`<test path>.<mode>.vpr`, relative to `build/vpr/`, sorted), so that
`scripts/lint_viper.py` can require every one of them, refuse an empty set and
refuse a `.vpr` file that the current MANIFEST does not list (a stale file).
`rejected` tests write no Viper and are not in the manifest. A test outside the
repository (the harness self-test) writes under `build/vpr/external/`.
`--update-golden` leaves verify tests alone, and does not clear `build/vpr/`.

### Helper headers

A test may include a header of its own (the driver verifies user headers). Such
helper files, of any C/C++ include suffix (`.h`, `.hpp`, `.inc`, `.def`, ...), live
only in a directory named `headers/` below the test area that uses them, for example
`tests/scalar/headers/`. They are never run as tests. Each one has no `// expect:`
line (it is not a test) and must be included, with `#include "..."` (the harness
passes no `-I`, so the name is resolved against the including file's directory,
for example `"headers/h.h"` or `"../headers/h.h"`), by a test of the area or by
another helper that a test includes; an orphan, which nothing verifies, is an
error. An include in a comment or in an `#if 0` block does not count (other
conditionals count as active). A verify test that includes a helper header with a
function body, directly or through other helpers, needs a position-map entry whose
file is that header (exit status 0 or 1), so a driver that skipped the header's
bodies fails; a header with only declarations, types and macros needs none. A source-like file anywhere else under `tests/` is still an error (it would
never run). `--check-headers` reports both.

### `.c` tests run in both languages

A `.c` test under `tests/scalar/` and under the three example directories of the M3
pages (`DUAL_MODE_DIRS`; the documentation examples are tests too) lists both modes,
`// mode: c23, c++20`, because the code is valid C and C++ and the dual run is what
checks it. A test that is only C says why: `// c-only: <reason>` (`.c` files only;
not with `c++20` in `mode:`). Negative tests are covered too.

### Categories for C and C++ tests

The categories of `// expect: error <category> @ L<line>` in a `.c` or `.cpp`
test are the driver's (`DRIVER_CATEGORIES` in `scripts/run_tests.py`, the table
of decision 0011 item 10). An unknown category is a header error. They are the
contract between the driver, the harness, the error-messages page and the tests,
and the harness self-test compares this table with the decision record.

| Category | Meaning | Reported at |
|---|---|---|
| `precondition` | a call may not satisfy a `requires` of the callee | the call |
| `postcondition` | an `ensures` may not hold | the `return` (or the closing brace of a `void` function) |
| `check` | a `check` may not hold | the `check` |
| `invariant-entry` | a loop invariant may not hold on entry | the invariant clause |
| `invariant-preserved` | a loop invariant may not be preserved by an iteration | the invariant clause |
| `overflow` | signed arithmetic may overflow | the operator |
| `division-by-zero` | a divisor may be zero | the operator |
| `shift` | a shift amount or a left-shifted value is out of range | the operator |
| `conversion` | a conversion to a signed type or to an enumeration may not fit | the conversion |
| `missing-return` | a non-`void` function may reach its end without `return` | the closing brace |
| `termination` | a termination measure (`decreases`) may be negative or may not decrease (decision 0013) | the loop's `decreases` clause, or the recursive call |
| `noreturn` | a function declared `[[noreturn]]` or `_Noreturn` may return (decision 0015) | the reachable `return`, or the closing brace |
| `uninitialized` | a local declared without an initializer may be read before it is given a value (decision 0019) | the read |

## Every back-end diagnostic has a test

`src/backend/diagnostics.def` lists the tags of the cerastes back end
(`CERASTES_DIAG(tag, severity, "message")`); the harness reads the tags from it
(a `cerastes-<tag>` in a header is validated against it). The harness self-test
requires every tag of that file to be named by at least one `frontend-error` or
`rejected` test under `tests/` or the documentation examples, with these exceptions, listed
with their reasons in `UNTESTED_BY_FRONTEND_ERROR` in `scripts/test_run_tests.py`
(an entry for a tag that a test names, or that the file no longer defines, fails
as stale):
- `contract_not_propagated`, `parameter_after_pack` and `result_name_on_void` are
  defense-in-depth checks that EDG's own checks make unreachable from source.
  `annotation_substitution_error` was one of them until the pin at P (2026-10-20): EDG now
  leaves an error node, without a diagnostic, in the substituted contract of a function
  template whose `forall` lambda takes `decltype(p)` of a function parameter, and the check
  rejects it (`tests/annotations/negative/forall_function_template_decltype.cpp`). A self-test runs `scripts/cerastes-fe.sh --c++20 --cerastes_checks_selftest
  --brief_diagnostics --display_error_number` on a contracted function template
  and a contracted variadic template, requires all four tags (`annotation_substitution_error` twice) plus two
  `unsupported_annotation_expression` errors (and no other error) with the
  option, and a clean run without it. The same option injects an error node into a substituted annotation tree (the
  `annotation_substitution_error` check) and makes the first tree of
  the walk read one parameter reference as `(unmapped)` and one node as visited
  twice (`(repeated node)`); `PrinterPathTests` checks both lines and both
  errors, and that they are absent without the option. The hook injects an error
  expression into the first node and an error type into the second, so both branches of
  `annotation_substitution_error` fire (two errors, `(error expression)` and `(error type)`);
- `source_scan_incomplete` is reached when the primary source is read from standard input
  (`cerastes-fe.sh -`), which a test file cannot be; `SourceFromStdinTests` runs it, and
  the same text from a file, which is clean (`STDIN_ONLY_TAGS`);
- `analysis_limit_reached` (a budget of a million shared expression nodes) is not
  reached by any reasonable source (EDG's own parser slows down first);
  `--cerastes_checks_selftest` lowers the budget, so `ChecksSelftestTests` shows it fire
  (`SELFTEST_ONLY_TAGS`).

`unsupported_annotation_expression` (a fallback node, a depth over 256, a lambda
body that is not one `return <expr>;`, an unmapped or repeated node) has
negatives for the first three. `PrinterPathTests` (real front end, from source)
shows each path of the walker's printer fire on a fixture, with a twin that
does not reach it: `(children not printed)` (a requires-expression in a
dependent contract), `(depth limit)`, `body <N statements>`, `tconst` kinds
other than `tpck_expression`, `contract from root template`, and the walker
warning `contract of <root> missing on <routine>` (with
`--cerastes_walk_selftest` on a contracted template, never without it).

## Refused options

The driver refuses some options before EDG runs (`--remove_unneeded_entities`,
C modes older than C23, GNU emulation `--gcc`/`--g++`/`--gnu_version`, and `-U` of
`__CERASTES__`, which the driver defines itself: decisions 0007 and 0008), with exit status 2 and a
single line, not with a diagnostic at a line of the file. A `frontend-refused`
test says so:

```
// expect: frontend-refused is not supported
// cerastes-fe-options: --remove_unneeded_entities
```

The harness runs, from the root, `scripts/cerastes-fe.sh <options> <test path>`:
the options of `cerastes-fe-options:` (shell-style words, each starting with
`-`) **replace** the mode option, and nothing else is added. One run, whatever
the file's suffix. The test passes only if the run is one of two shapes, and
the message contains the substring:

- **the driver's refusal**: exit status 2, nothing on stdout, and exactly one
  non-empty line on stderr, which starts in column 0 with `cerastes-fe: error: `
  (`REFUSAL_PREFIX`; src/driver/main.cpp prints it for every refusal);
- **EDG's own command-line error** (an unknown option): exit status 4, nothing
  on stdout, and exactly `Command-line error: <message>` (or `Command-line
  error #N: ...`), `1 catastrophic error detected in this compilation.` and
  `Compilation terminated.`.

Anything else fails: a run that compiles (status 0), an ordinary compilation
with errors (also status 2, but not one line with the prefix), another status,
a crash, a timeout, anything on stdout, an extra stderr line. The expected
substring is matched in the message, not in the prefix. The kind is not
combined with other outcome kinds, `golden`, `native-ok` or `mode:`;
`cerastes-fe-options:` is valid only with it; `.c` and `.cpp` only. It counts as
a must-fail test (it can live under `negative/`), runs the EDG-built front end
(so the tree and stamp checks apply), and `--update-golden` leaves it alone.

## Ordinary compilers (`native-ok`)

A test with `// expect: native-ok` is also compiled, for each of its modes and
by each compiler, with

```
<compiler> -x c++ -std=c++20 -Wall -Wextra -Werror -U__CERASTES__ -I <include dir> -c -o /dev/null <file>
<compiler> -x c   -std=c23   ...        (mode c23)
```

The mode, not the suffix, picks the language: a `.c` test that lists
`// mode: c++20, c23` is compiled as C++ (`-x c++`, by `clang++` and `g++`) in its
`c++20` run and as C (`-x c`) in its `c23` run, so the dual-mode scalar tests of
`tests/scalar/` are checked by the compiler of each language.

and passes only if the compiler exits with status 0 and prints nothing on
stdout and stderr. The ordinary compilers never define `__CERASTES__` (the
cerastes front end does), and `-U__CERASTES__` makes sure: that is what makes
`<cerastes.h>` expand its annotations to nothing. The compilers come from `build/env.sh`, which is
[authoritative](#the-environment): `CERASTES_CLANG_CC`/`CERASTES_CLANG_CXX`
(Apple clang, found with `xcrun`; run by its real path the harness sets
`SDKROOT` for it if it is not set) and `CERASTES_GCC_CC`/`CERASTES_GCC_CXX` (the
highest-numbered `gcc-NN`/`g++-NN` found, or empty strings; an empty value
exported by the shell cannot switch GCC off). Without a GCC the run prints one note,
`native-ok: GCC not installed; Apple clang only`; a GCC that `env.sh` names but
that cannot be run fails the run. The summary line ends with the per-compiler counts
(`native-ok runs: Apple clang 6, GCC 6`). A native run happens even when the
test's own outcome fails; `--update-golden` and `--check-headers` never compile.

An unknown namespaced attribute is a warning for both compilers, so with
`-Werror` a file with `[[unreg_ns::x]]` is never native-ok; that is why
the annotation layer's positive tests spell their attributes through the macros
of `<cerastes.h>`.

**Anti-vacuity canaries.** Once per compiler and language per run, before any
test, the harness checks that the compiler can vouch for a file: its
`--version` banner says Apple clang (resp. GCC, and not clang), a snippet that
is fine compiles silently with the same flags, and both a snippet with an unused
variable and one with an unknown scoped attribute (`[[unreg_ns::x]] int v;`) are
rejected by them. A canary prints as `canary native-ok [<compiler>,
<language>]`; when it fails, every native run of that compiler fails with it,
so a compiler that accepts everything cannot make files pass.

**A native-ok test cannot switch its own check off.** A `#pragma clang|GCC
diagnostic ignored|warning`, a `#pragma clang|GCC system_header` or a
`#define __CERASTES__` in a native-ok test is a header error (`#undef
__CERASTES__`, `diagnostic push|pop|error` stay allowed).

**Policy.** `native-ok` is required (a header error, also with
`--check-headers`) of every positive test (`golden`, later `verified`) in the
directories listed in `NATIVE_POLICY_DIRS` in `scripts/run_tests.py`:
`tests/annotations`, `tests/scalar`, `doc/modules/ROOT/examples/annotations`,
`doc/modules/ROOT/examples/ordinary-compilers` and the examples of the M3 pages,
`doc/modules/ROOT/examples/verifying`, `doc/modules/ROOT/examples/error-messages` and
`doc/modules/ROOT/examples/integer-semantics` (with their subdirectories), and
of every positive test, wherever it is, whose source has an
`#include <cerastes.h>` (or `"cerastes.h"`) line. Negative tests are exempt. `expect: frontend-ok` is a header error in these
directories (stock cpfe does not define `__CERASTES__`, so the annotations
would not even be parsed, and the test would escape both the twin rule and
native-ok).

**No code only the verifier sees (owner, 2026-10-22; decision 0019 item 11).** A native-ok test may not hide code
from the ordinary compilers: no `#ifdef __CERASTES__` around code of the program itself (the macros of `<cerastes.h>`
are the one use of `__CERASTES__`, and they expand to annotations only). M4.2b2 had accepted one exception, a
never-taken `case 4:` over an enumeration under `#ifdef __CERASTES__` that `-Wswitch` refuses; the verifier now rejects
such a switch itself (`cerastes-case_not_in_enumeration`, `cerastes-enumerator_not_handled`), so the program is not
valid for cerastes and must be written without it. A shape the compilers refuse is a negative test (a `rejected` or an
`error` one), which is exempt from native-ok. This is a convention reviewed by hand, not a harness rule.

## Time limits

Each run of the front end (`cerastes-fe`, `cpfe`) has a limit of ten minutes, `FRONTEND_TIMEOUT_S` in
`scripts/run_tests.py`, and a run of `cerastes verify` or Silicon the same. The largest front-end test,
`edg-smoke/libcxx_headers_cerastes_fe.cpp` (the libc++ headers in the cerastes configuration), takes about 70
seconds of CPU, which a loaded machine (a parallel build, other test runs) stretches well past the two minutes
that the limit used to be. A limit is there to catch a hang, never to measure speed: reaching it always
fails the test, whatever the expectation.

## Parallelism (`-j`, `CERASTES_JOBS`)

`run_tests.py` and `mutate.py` run tests in parallel, each with a pool of `-j N` workers (`--jobs`). The
default is `CERASTES_JOBS` if it is set, else half the CPUs (at least 1: `default_jobs()` in
`scripts/run_tests.py`). The variable is read from the shell, never from `build/env.sh` (a value there is
ignored, and `bootstrap.sh` does not write one). It must be a positive decimal integer (`3`, `10`): the empty
string, `0`, a sign, a leading zero, a space or anything else is a usage error (exit 2) that names the variable,
so that a typo cannot silently change a job count. An explicit `-j` always wins and does not read the variable,
so a bad value stops only a run that needs the default (`--check-headers` and `mutate.py --validate` run
nothing in parallel). `scripts/check.sh` passes no `-j`, so the variable sets its job count too (the VIR
self-test run included: the self-test guard rejects any `-j` on that line). Each worker may start a Silicon JVM,
which is why CI sets `CERASTES_JOBS=3`: three fit the 7 GB of its runner, eight do not. `scripts/walker_oracle.py` keeps its own default
(`min(8, cores)`, for its front-end runs only) and does not read `CERASTES_JOBS`.

## Mutation testing (`scripts/mutate.py`)

`python3 scripts/mutate.py` (45 to 90 minutes at `-j 8` for `tests/scalar`: run it alone, and give it a
time cap you can live with, since it logs as it goes, see Progress below) mutates every `verified` test one place at a
time (flip a relational operator, a literal plus or minus one, drop one operand of `&&`, swap `&&` and `||`,
delete a statement; and, in code only and where the value of the expression is used, `drop-store` (`(x = e)`
becomes `(e)`, `y = x = a` becomes `y = a`), `swap-incdec` (`x++` and `++x` swapped, `++` and `--` swapped) and
`drop-comma` (`(a, b)` becomes `(b)` when `a` has an effect, also in a `for` init or step clause and as an
expression statement; never an argument or declarator separator, and no mutant at a statement that discards the
value, such as `x = 1;`, `i++;` or `(void)x++;`)) and verifies each mutant. A mutant that is rejected by the verifier (exit status 1) is
killed; one that still verifies survived, which means that the test would not notice the change. Every
survivor must be killed (by strengthening the test), or listed with one reason in `tests/mutants-equivalent.txt`
(`id<TAB>reason`, plus a `# reviewed:` line), or be in the safe-direction bucket. The run ends with zero
unexplained survivors, and a test none of whose mutants was killed fails the run.

**Why a safe-direction bucket (owner decision, 2026-10-16).** A sound verifier must accept a program that
meets a *weaker obligation than the original*. Changing a contract so that it is easier to meet gives such a
program: a stronger `requires` (the body has less to prove; a caller in the file may fail it), a weaker
`ensures` (the body proves more than the clause says; a caller may rely on it) or `check` (the body proves
more than the clause says, and nothing relies on a check). A
verifier that rejected these mutants would be unsound or incomplete the other way, so a surviving one says
nothing about the test, and listing thousands of them would bury the survivors that do. They are counted in
their own bucket, `safe-direction`, which the run reports for each test and in total; they are neither
required to be killed nor listed (a listed one is a stale entry). The direction is decided by comparing the two
clauses on a grid of values (`clause_relation` in `scripts/mutate.py`). The grid is mathematics on plain
integers, so a mutant is bucketed only when the grid models C for it; otherwise it is `unknown`, never
bucketed, and must be killed or listed. A mutant is bucketed only when all of these hold:

- every variable of the clause (original and mutant) is a parameter of the enclosing function whose declared
  type is a signed builtin integer type (`signed char`, `short`, `int`, `long`, `long long`, with an optional
  `signed` or `const`; not `bool`, plain `char`, anything unsigned, a typedef, a pointer or a reference); the
  result name of an `ensures` counts only if the return type is one; a `check` name that the body declares
  again does not count; a contract of a bodyless declaration is `unknown`;
- the clause is linear: no `*` at all (a literal coefficient is not monotone: `3 * x + y < 10` against
  `4 * x + y < 10` is incomparable), no `/`, `%`, `<<`, `>>`, bit operation, `?:`, cast, call, subscript or
  unsigned literal suffix;
- Python and C read the clause alike: at most one comparison between two `&&`/`||` or brackets (`x < r < 1`,
  `r == x > 0`), the operand of `!` is followed only by `&&`, `||`, `)` or the end (`!r < 1`), and a bracketed
  group with `&&` or `||` of its own is not an operand of a comparison or of arithmetic;
- the mutation is not an `==`/`!=` flip;
- for a stronger `requires` or a weaker `ensures` (not a `check`), the function's name occurs nowhere else in
  the file, except in its own declarations and definitions: a call, a recursive call, its address, a mention in a
  contract, in a `#define` body (or any directive but `#include`), in a default argument or in an initializer
  is a use (comments and strings are not read; a directive the script cannot lex makes every name used);

`assert(e);` (decision 0015: a failing assert is a `check`) is a `check` clause: a mutant inside the argument
of an `assert(...);` statement is bucketed when the argument gets weaker, under the rules above as for a `check`,
and only when the argument is pure (no call, `++`, `--`, assignment or comma). A stronger argument is never
bucketed. Only the macro is meant: `static_assert` and names that merely contain `assert` are not.

**Why the bucket does not ask whether the mutated clause is well-defined (owner, 2026-10-21: "verifier-checked").**
A bucketed mutant is a correct program only if its mutated clause is well-defined, and a dropped conjunct, a swapped
`&&`/`||` or a relop or literal change can weaken a guard (`x != 0 && 100 / x > 1` becomes `100 / x > 1`). The bucket
does not check that: a surviving mutant has already passed cerastes' own well-definedness check of contract arithmetic
(decision 0010 item 11: contract arithmetic has its C meaning, overflow and division by zero included, checked by the
emitted `__wf` methods and the clause's own checks), so an ill-defined survivor would be a bug of the verifier, not a
hole in the test. That check is pinned by must-fail tests, which a weakened verifier would fail:
`tests/scalar/negative/contract_wf_divzero.c` and `contract_wf_overflow.c` (twin `tests/scalar/contract_wf.c`), and,
with the guarded originals in `tests/scalar/guard_kept.c` as their must-verify twin, the contracts written as a mutant
would leave them: `guard_dropped_requires.c` (a requires, division by zero), `guard_dropped_ensures_later.c` (an
ensures whose guard sat in an earlier ensures that lost it, division by zero in the later one),
`guard_dropped_overflow.c` (`r < 2147483647 &&` dropped before `r + 1`, overflow), `guard_dropped_ensures_later_overflow.c`
(the same for the later ensures), `guard_swapped_requires_overflow.c` (`||` swapped to `&&` in a requires, overflow),
`guard_changed_invariant.c` (a loop invariant's guard changed, division by zero at entry) and
`measure_loop_overflow_later.c` (a measure that overflows; invariants and measures are never bucketed but are clauses
too). A weaker `check` or `assert` never evaluates an operand in a state where the original did not (they are judged
in the state at their line, where what the original proved is still known), so it cannot become ill-defined;
`guard_swapped_check.c` and `guard_swapped_assert.c` (`||` swapped to `&&`, division by zero) pin the same check for
the stronger form that the bucket never accepts, as defence in depth.

"Only when the grid models C" is not a proof (the grid is a handful of values per variable), so the rules above
fail closed: when in doubt a mutant is `unknown` and must be killed or listed.

`python3 scripts/mutate.py --list [paths]` prints the mutant ids and exits without running anything.
Everything else still has to be killed or listed: a weaker `requires` (a caller may now
break the body), a stronger `ensures` or `check` (the body may not prove it), any change of an `invariant` or a
`decreases`, a mutant that leaves a clause's meaning unchanged, and every mutant of code, never bucketed.

**Progress.** A run writes one line to stderr, flushed, when each verifier run finishes (in finishing order):
`mutate: [k/N] <mutant id or "<path> (original)"> [<mode>]: <outcome>`. k and N count the originals and the
mutants apart (outcomes `verified`, `does not verify`, `rejected`, `tool failure` for an original; `killed`,
`survived`, `stillborn`, `tool failure` for a mutant). Stdout is the report and does not change. A run stopped by a
time cap still leaves its outcomes in the log: `python3 scripts/mutate.py -j 8 2> build/mutate.log`.

**The equivalence list and the selection.** A run over some of the tests checks only the entries whose file it
selected. `--validate` (the `check.sh` step, `tests/scalar` by default, a few seconds of work) also generates every
file the list names outside the selection (the doc examples) and checks all entries against them; an entry whose file
does not exist or is not a `// expect: verified` C or C++ test is stale. A run does not check the entries of
files it did not select, and `--validate` cannot tell whether an entry survives: the acceptance run must name every
file the list names (the doc examples among them: `python3 scripts/mutate.py tests/scalar doc/modules/ROOT/examples/...`),
or those entries are never checked for survival.

## The environment

`scripts/run_tests.py` reads `build/env.sh` the way the scripts do, by sourcing
it: **the file is authoritative**. Every variable it defines (`CERASTES_FE`,
`CERASTES_EDG_GEN`, `CERASTES_INCLUDE_DIR`, `CERASTES_GCC_CC`, ...) takes its
value from the file, whatever the shell exported. An inherited value that
differs is ignored, with one note per variable on stderr (`ignoring the inherited
NAME=... ; build/env.sh defines NAME=...`), so that a stray variable cannot
silently drop GCC, swap the tag tables used to validate `frontend-error` tests,
or change the include directory. Variables the file does not define (`PATH`,
`SDKROOT`, ...) are inherited. The one exception is the selection of the EDG
tree: `CERASTES_EDG_SRC` is read from the shell, and checked against what
`env.sh` was written for (see [The EDG tree](#the-edg-tree-decision-0005)). The
harness self-test points the harness at fixture env files, not at the process
environment.

`CERASTES_JOBS` (the default `-j`, [Parallelism](#parallelism--j-cerastes_jobs)) is also read from the shell
and not from the file.

## Twins

Every test under `tests/annotations/negative/` (`NEGATIVE_DIR`, with its
subdirectories) and under `tests/scalar/negative/` (`SCALAR_NEGATIVE_DIR`; both
in `NEGATIVE_DIRS`) is the must-fail twin of a positive test and names it:
`// twin: tests/annotations/<name>.cpp` or `// twin: tests/scalar/<name>.c`, a
repository-relative path. A missing
`twin:` line is a header error, and so is a twin that does not exist, is itself
under `negative/`, is the test itself, has a bad header, or has an outcome other
than `golden` or `verified`. A `twin:` line in any other test is checked the
same way. The harness does not compare the two files; the line records, and
checks the existence of, the pair that the must-fail twin rule asks for.

A test under `negative/` must be a must-fail test: it expects at least one
error (`frontend-error`, `error` or `rejected`). `golden`, `verified`,
`frontend-ok` and warning-only tests are header errors there; tests that
document a warning live in `tests/annotations/warnings/`.

The rule also runs the other way (invariant 2: every encoding rule has a
must-fail test), in each of the two areas (`TWIN_AREAS`). Every positive test
(`golden` or `verified`) under `tests/annotations/` (resp. `tests/scalar/`)
outside its `negative/` must be named as the `twin:` of at least one test under
that area's `negative/`, or say why not with
`// twin-exempt: <reason>` (a non-empty reason; not allowed together with
`twin:`, on a test that is not positive or under `negative/`). A positive test
that no negative test names is reported as a header error (`bad header:`),
also by `--check-headers`, whatever subset of the tests was selected: the
check reads the whole tree of an area whenever a selected test is in it.

In `tests/scalar/` (`ERROR_TWIN_AREAS`) the twin must expect an `error` and must run
in every mode of its positive (a twin that runs in c23 only says nothing about the
c++20 run of a dual-mode positive; two twins, one per mode, are enough): a
`rejected` twin shows that a construct is refused, not that the encoding rule of
the positive test is checked, so it does not count (the header error then says
that the test has only a `rejected` twin). The acceptance count of pairs counts
only those: `python3 scripts/run_tests.py --count-error-twins` prints the number
of positive tests of `tests/scalar/` with at least one `error` twin (a positive
test with `twin-exempt:` is not counted).

**Pinning the size of a generated directory.** `python3 scripts/run_tests.py --expect-passed N
<dir>` fails unless exactly `N` tests passed and none failed. `scripts/check.sh` uses it for the
cases that `build/cerastes-vir-selftest` generates (`N` is the count the tool prints, which must
be at least `VIR_CASES_MIN`), so a smaller set of cases, or a file that is not run, cannot pass.

**What counts as Silicon's verdict.** The harness (and the driver, which must agree with it, see
`ClassifySiliconAgreementTests`) accepts Silicon's output only as the banner, exactly one verdict
line and its error list: a stray line, a second banner or verdict, or a missing banner is "no
verdict", never a pass.

## The EDG tree (decision 0005)

Tests that run a tool built from the EDG tree (`frontend-ok`, `golden`,
`frontend-error`/`-warning`, `frontend-refused`, and the `verified`, `error` and
`rejected` tests of `.c` and `.cpp` files) first check that `build/env.sh` was written for the
tree that `CERASTES_EDG_SRC` of the current shell selects (`third_party/edg` when
it is unset or empty), compared by real path, and stop with exit status 2 and
`build/env.sh is for <X> but CERASTES_EDG_SRC selects <Y>; run
scripts/bootstrap.sh` when it does not. `scripts/cerastes-fe.sh` and
`scripts/cpfe.sh` do the same. Before such a test runs, the harness also
verifies the active kind's build stamp against the tree and the cerastes
configuration as they are now, as `edg_tree.py describe --verify-build` does,
and stops with exit status 2 (`run scripts/bootstrap.sh`) when the binary is
stale. `--check-headers` does not need the tree. A
relative path in `CERASTES_EDG_SRC` is refused with exit status 2 and a message
(`... is a relative path; use an absolute path`) by `run_tests.py`,
`cerastes-fe.sh`, `cpfe.sh`, `bootstrap.sh` and `edg_tree.py`.
`scripts/edg_tree.py` (`resolve`, `stamp`, `check`, `describe --verify-build`)
implements the guards that `scripts/check.sh` runs first and last; the harness
self-test runs it on temporary repositories (a nested repository at
`third_party/edg` with a fake gitlink and fake `origin/cerastes` and
`fork-local/cerastes` branches, override trees)
and checks that `check.sh` runs both guards as active, ordered steps that
cannot be ignored. The guards, beyond a clean `third_party/edg` at its gitlink
with the fork's URL (`https://github.com/mborland/compiler.git`, in
`.gitmodules` and as the submodule's `origin`; upstream's URL fails):
- the pin is a commit of the owner's fork (decision 0009): the gitlink must be
  an ancestor of the submodule's `refs/remotes/origin/cerastes` (pushed; the
  report line is `EDG tree: fork third_party/edg @ <8> (gitlink, clean)`) or of
  `refs/remotes/fork-local/cerastes` (the local clone, added by a session as the
  remote `fork-local`; the report line then says `NOT YET PUSHED to
  origin/cerastes` and a warning says the owner must push the fork's `cerastes`
  branch before cerastes `main`). A commit on neither, or missing refs, fails
  with `git -C third_party/edg fetch origin` / `fetch fork-local`; `origin/main`
  and other branches do not count. The kind label `upstream` (the
  `resolve` output, `CERASTES_EDG_KIND`, the `build/` layout) is kept as the
  name of the pinned tree; only the report line says `fork`;
- an override needs an absolute path, a git work tree root with `src/il_def.h`,
  not the submodule or inside it, and HEAD of the repository must be a named
  branch other than `main` (not detached);
- the build stamp has five lines (`tree`, `head`, `branch`, `dirty`, and
  `config <sha256 of cmake/EdgFrontEnd.cmake and src/backend/edg_il.h>`), so a
  binary built under an older cerastes configuration does not verify;
- every git command runs without `GIT_DIR`, `GIT_WORK_TREE`, `GIT_INDEX_FILE`
  and the other variables that redirect git (`bootstrap.sh` unsets the same
  list, `GIT_REDIRECTS`, which the self-test compares; a stub `git` logs what
  each call of a real `bootstrap.sh --build-only` saw).

## Documentation examples

Every code example in the user documentation (`doc/`, Antora; invariant 8) is a
test. The examples live in `doc/modules/ROOT/examples/` (subdirectories are
fine) as ordinary `.c`, `.cpp` and `.vpr` files with the usual `// expect:`
header, and `scripts/run_tests.py` discovers them by default (`DEFAULT_PATHS`),
with the same rules as `tests/`: skip markers are not allowed there, a source
file with a non-test suffix and an orphan golden file are errors, and a bad
header fails.

A page shows an example only through an include of a tagged region, so the
expectation header never appears on the page. The block holds exactly one
include and nothing else:

```
[source,cpp]
----
include::example$installation/hello.cpp[tag=hello]
----
```

The example has the tag lines after its header, written as comments:
`// tag::hello[]` and `// end::hello[]`. Put them outside `#if`, `#ifdef`,
`#ifndef` and `/* */`, or the page could show code that is compiled out.

Two checks guard this. They are complementary, because a line-based reader
cannot know every AsciiDoc form:

**1. The source lint**, `python3 scripts/check_doc_examples.py` (a
`scripts/check.sh` step, and a step of the CI build job before the build; exit
0 clean, 1 problems, 2 when the doc component is missing or `doc/modules/` has
a module other than `ROOT`; `--root DIR` checks another tree). It reads
`nav.adoc`, `pages/**/*.adoc` and `partials/**/*.adoc` line by line and reports
`<file>:<line>: <message>` for:

- a source block in a language cerastes reads (`c`, `cpp`, `c++`, `cxx`, `h`,
  `hpp`, `vpr`, `viper`, case-insensitive, with a few other spellings) whose
  body is anything but one `include::example$<file>[tag=...]` line, where the
  file has a test suffix (`.c`, `.cpp`, `.vpr`). A `partial$`, an attachment, a
  non-test file or inline code is an error. The forms recognised are:
  `[source,cpp]`, `[,cpp]`, `language=cpp`, `----`, `--`, fenced blocks, the
  paragraph form without delimiters, blocks that start on a table cell line
  (`a|[source,cpp]`), with a title, anchor or blank line before the block. This
  is a best-effort line reader, not an AsciiDoc parser; the `--html` check
  below is the backstop for the forms it cannot see;
- code or text shown without a declared language, which is how code could hide
  from the check above: a `----` listing, fenced block or source paragraph that
  does not declare `[source,<language>]` (the language must be a cerastes one,
  included from a test, or one of `sh`, `console`, `text`: `OUTPUT_LANGUAGES`,
  extended only by a reviewed change); a literal block (`....`); a paragraph
  that starts indented (a literal block, also after a list `+` continuation,
  in a description list or in a table cell; an indented list item is fine); the
  `[verse]` and `[pass]` styles; any `subs=` on a block attribute line (it
  can switch off escaping and emit raw HTML); `l|` and `m|` table cell styles and `cols`
  specs with `l` or `m`; a `++++` passthrough block; a `pass:[...]` whose
  content has `<` (raw HTML); a `:source-language:` entry, and a
  `source-language` attribute in `doc/antora.yml` or a playbook in `doc/`;
- an `include::example$...` without `tag=` or `tags=`, with `lines=` (which
  wins over the tags and shows the header), with an empty tag, or with a tag
  that is not a plain name (`*`, `**` and `!x` select untagged lines);
- an include whose target does not exist, is not `example$` (or `partial$`),
  is not a valid test (bad or missing header, or not a test suffix), uses a tag
  that the file does not define, has a tagged region that contains an
  `// expect:` or `// mode:` line, or has a tag directive inside an `#if`,
  `#ifdef`, `#ifndef`, a `/* */` comment or unclosed parentheses (a macro
  call's arguments). A tag directive must be a `// tag::x[]` or `// end::x[]`
  line comment alone on its line, and not follow a line ending in a backslash
  (a continued `#define` would hold it). A raw string literal (`R"`) anywhere in
  an example, and a `//` comment ending in a backslash, are errors too: they
  can hide a tag directive in code that is not compiled;
- walker listings (rule g). The listing the IL walker prints for an example
  is the example's `.golden` file, so it can be shown and tested. An
  `include::example$<test>.golden[]` is allowed only as the whole body of a
  `[source,text]` block (`----`, nothing else in it), with an empty attribute
  list (no `tag=`, `lines=`, `indent=`, `opts=`), for the golden file of a
  `golden` test under `examples/` (`<test>.golden`, or `<test>.<mode>.golden`
  for a test with several modes) whose test the **same page** includes through
  a tagged region: a listing only appears next to the code that produced it.
  The golden must not have a line ending in a callout marker (`<1>`), which
  Asciidoctor would turn into a callout (a real build shows it; includes,
  conditionals and tag directives inside an included file are left alone).
  Any other include of a golden file is an error, and so is a line that looks
  like walker output in a `text`, `console` or `sh` block that is not such an
  include, or in inline code (`` `...` ``, `+...+`) in prose: a listing written
  by hand, even an excerpt, could not be told from a tested one. A line looks
  like walker output when it contains, anywhere (so a prefix such as `out> `
  does not hide it), a source position (` @ L<n>:<c>`, ` @ -`), `type_kind=`,
  `end walk:`, `op eok_...`, `stmt stmk_...`, `tconst tp...`, `node enk_...`,
  `attr [[ns::...` or a `const V type=T` leaf; or when it starts, after such a
  prefix, with a line kind that has no position (`routine F plain|prototype|
  instance|specialization`, `returns T`, `param N x: T`, `template F args`,
  `constraint <kind>`, `field Q: T`, `binder N: T`, `this: T`, `body return`,
  `arg N binder|const|@`, or one of the walker's `warning:` lines). The
  self-test checks that every line of every walk in `tests/il/*.golden` is
  recognised;
- a file under `doc/modules/*/examples/` that is not a test or its `.golden`
  file, and any C/C++/Viper source file (by suffix) elsewhere under
  `doc/modules/` (`partials/`, `attachments/`, `pages/`...);
- an orphan: a test under `examples/` that no page includes (an include inside
  a `////` comment block does not count);
- a `++` in text outside `pass:[...]`. It opens an AsciiDoc passthrough that
  silently swallows text up to the next `++`, and Antora gives no warning. A
  real build (tested in the harness self-test) shows that one alone is left
  alone, two garble the paragraph (`libc++`, `g++` and `-std=c++20` too, in
  backticks too), `+...+` does not protect them, and `pass:[...]` does. So write
  `pass:[C++]` (and `pass:[C++]20` for a version), `pass:[libc++]`,
  `` `pass:[--c++20]` ``. It applies to prose, titles, headings, list items,
  table cells, inline code, link text and attribute entries, not to `----` and
  fenced blocks, source or listing paragraphs, `////` blocks, `//` comments and
  include lines. A block attribute line is checked, but its language
  (`[source,c++]`) is not;
- a doubled formatting mark (`__`, `**`, `##`) that occurs twice in a paragraph
  outside `pass:[...]` and `+...+` literal spans (counted per paragraph, per
  list item or description-list entry and per table cell): the pair formats what
  is between them, across code spans (two `` `__algorithm` `` spans on a line render
  as an italic run). One alone is safe (tested against a real build). Wrap it:
  `` `+__algorithm+` ``.

**The error-messages page (rule h).** `pages/error-messages.adoc` has exactly one
`== <category>` section per category of `DRIVER_CATEGORIES` (the driver's, see
[Categories for C and C++ tests](#categories-for-c-and-c-tests)), titled with the
category name, and each section includes at least one example whose header expects
`error <that category> @ ...` (a `verified`, `rejected` or other-category example does
not count, nor an include in a comment or in another section). A missing category, a
category twice, a section that is not a category and a section without such an
example are problems. Inactive while the page does not exist and no `verified`,
`error` or `rejected` C or C++ test is under `tests/`; once one is, a missing page is
a problem (the trigger is `scripts/lint_viper.py`'s).

**2. The built-site check**, `python3 scripts/check_doc_examples.py --html
build/doc`, run after `scripts/build_docs.sh` (in `check.sh` and in the docs
workflow's build job). It does not read the pages: it classifies every `<pre>` of the
generated HTML (and every `<code>` with a `language-X`, `lang-X` or `data-lang`):

- a block in a cerastes language must, after unescaping, equal a tagged region
  of a test under `examples/` (the lines of the chosen tags in file order, tag
  lines dropped; whitespace at line ends, common indentation and blank lines
  around the code are ignored), and the site must have at least as many such
  blocks as the page sources declare (so a UI change that renames the class
  cannot make the check pass without looking at anything);
- a block in `sh`, `console` or `text` is fine, except that a block with
  walker output (as in rule g) must equal a golden file of a golden test under
  `examples/` after the same normalisation, and the site must have at least
  as many such blocks as the page sources have golden includes, and inline
  `<code>` with walker output is an error;
- anything else is an error: a `<pre>` without `<code>` (a literal block, verse,
  raw HTML), a `<code>` without a language class, any other language, `lang-cpp`
  and so on. Page chrome is not exempt (the default UI's own pages have no
  `<pre>`; a future one would need a precise rule and a test);
- broken markup is an error: a backtick in the text outside `<pre>` (a code
  span that was not rendered) and an inline element closed while one inside it
  is still open (what two `__` code spans render to).

Code that the source lint misreads usually fails here, but not always:
`--html` cannot see raw HTML that looks like text (a `[subs=none]` paragraph
with a `<div style="white-space:pre">`), a `++` pair that eats text, or `**`
pairs (they render as well-formed HTML). Only the source lint catches those,
so the two checks are both required.

`check.sh` runs the examples as tests, both checks, and the docs build. Two
workflows run in CI. `.github/workflows/ci.yml` runs `scripts/ci.sh` (the
runner's setup and toolchain assertions) and then the full `scripts/check.sh`,
so the examples run as tests there too, on GitHub's `xcode-27` runner with
`CERASTES_JOBS=3` (decision 0016). The harness self-test pins the active text of both files:
`ci_workflow_problems` and `ci_sh_problems` compare every active line (comments and blank lines dropped) with
a pinned copy, so an extra step, an extra command, a `break` in a loop or a reassigned constant fails it;
only comments, the `uses:` SHAs, the step and job names, `timeout-minutes` and `CERASTES_JOBS` may change in
ci.yml, and only comments in ci.sh. Structured rules (the triggers, `contents: read`, one job with only five keys,
the runner, SHA-pinned actions, the checkout options, plain `scripts/ci.sh` then `scripts/check.sh` steps;
every toolchain assertion as a pinned line, in order, with no early `exit` or `|| true`) give the diagnostics.
The docs workflow (`docs.yml`) still builds
and deploys the docs: it runs the source lint, `scripts/build_docs.sh` (which
fails on any Antora warning) and the `--html` check, but not the examples as
tests, because those need the EDG and Silicon toolchain. The harness self-test
has a twin for each rule; it checks that `check.sh` and the docs workflow run the checks
as active, ordered steps that cannot be ignored (`|| true`, `set +e`,
`continue-on-error`, a comment); and an end-to-end twin builds a temporary copy
of `doc/` with Antora to show that a broken xref or include tag fails the
build, that pages which evade the source lint (a partial in a cpp block, a
non-test include, a table cell, another module, a component-wide
`source-language`, `++++` or `[verse]` or `l|` or an indented literal showing a
`<pre>`, `pass:[<pre>]`, ...) still fail `--html`, and which markup garbles a
page in a real build.

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
alone (never a documentation example directory). A marker anywhere else, including `tests/il`, is an error (exit 2) that
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

Categories for C and C++ tests are the driver's: see
[Categories for C and C++ tests](#categories-for-c-and-c-tests).
