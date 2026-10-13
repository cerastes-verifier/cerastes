# Tests

Run everything with `python3 scripts/run_tests.py` (or a list of files and
directories). `scripts/check.sh` runs it after the harness self-test
(`scripts/test_run_tests.py`).

| Directory | What it checks | Runner |
|---|---|---|
| `viper-smoke/` | Silicon and Z3 work, and the harness reads Silicon's errors correctly | Silicon |
| `edg-smoke/` | The pinned EDG front end parses C++20 and C23 against the system headers | `scripts/cpfe.sh` |

Later milestones add `annotations/`, `scalar/`, `templates/`, `concepts/`,
`memory/` and `differential/` (see plan.md).

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
- **`mode:`**: the language modes to run a C or C++ test in, comma-separated
  or on several lines. Each mode is a separate test run. The default is
  `c++20` for `.cpp` and `c23` for `.c`. Not allowed in `.vpr` files.

Line numbers are 1-based lines of the test file itself. Solver timeouts,
crashes, parse or type errors and messages the harness cannot classify always
fail the test, whatever the expectation.

**Must-fail twin rule:** every must-verify test that exercises an encoding
rule has at least one twin that must fail for the right reason at the right
line.

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
