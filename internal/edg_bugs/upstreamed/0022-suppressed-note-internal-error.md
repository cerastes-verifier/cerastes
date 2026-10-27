# 0022 — Suppressing the number of a constant-evaluation note is an internal error: "determine_severity_code: bad severity"

- **Labels:** `bug`
- **Found:** probe P27 of cerastes M4.2b2 (2026-10-22). Reproduced on upstream `main` `ec2f12cffa` in session M4.2c
  the same day.
- **Status:** fixed on branch `0022` (`3b05dd5c43`, one commit on `ec2f12cffa`), merged into the fork's `cerastes`
  by the owner (`f0ed5525af`, 2026-10-22) and pinned by cerastes (M4.2c); not filed. The full box suites at the merge
  are still to run. The commit's Changes entry still carries `[GH #?]` until the issue has a number.
- **Upstream issue:** not filed

## Summary

When constant evaluation fails in C++, the error "expression must have a constant value" (#28) carries the
interpreter's diagnostics as notes, for example "shift count is too large" (#63), "shift count is negative" (#62),
"division by zero" (#39) and "integer operation result is out of range" (#61). Each note is created as an
`es_more_info` diagnostic, and `create_diagnostic_entry` (`error.c`) applies `check_for_overridden_severity` to it as
to any other diagnostic. With `--diag_suppress` (or `#pragma diag_suppress`) naming a note's number, the note gets
severity `es_none`. Displaying the error then reaches `determine_severity_code` with `es_none` (`error.c` line 3023)
and stops with an internal error. It happens in the C++ constant-expression contexts that report through error #28:
a `constexpr` variable, an enumerator, `static_assert`, an array bound, a template argument, `if constexpr`,
`alignas`, a bit-field width (not a `case` label, which reports #39 itself, and not a narrowing template argument,
#2361). Only the text output fails: the SARIF output (`--output_mode=sarif`) does not compute a note's severity and
lists the suppressed note as a related location. C has no such notes and is not affected.

## Reproducer

```c++
constexpr int z = 1 << 32;
```

Command: `cpfe --c++20 --diag_suppress=63 shift.cpp` (native base, `--clang_version=210000`, the scraped system
include directories).

**Actual:**

```
"shift.cpp", line 1: internal error: assertion failed:
          determine_severity_code: bad severity (error.c, line 3023 in
          determine_severity_code)

  constexpr int z = 1 << 32;
                    ^

1 error and 1 catastrophic error detected in the compilation of "shift.cpp".
Compilation aborted.
```

The exit status is 134 (Abort trap). The same happens for `constexpr int z = 10 / 0;` with `--diag_suppress=39`, and
for `#pragma diag_suppress 63` before the declaration instead of the option.

**Expected:** the error without the suppressed note, exit status 2:

```
"shift.cpp", line 1: error: expression must have a constant value
  constexpr int z = 1 << 32;
                    ^

1 error detected in the compilation of "shift.cpp".
```

(The front end also warns, as without the option, that `z` is declared but never referenced.) Without
`--diag_suppress` the output is the error with `note: shift count is too large`, as now.

## Confirmation

Upstream `ec2f12cffa`, native macOS arm64 `macos-arm-clang-debug`, `--c++20`:
- `--diag_suppress=63` (shift), `--diag_suppress=39` (division) and `--diag_suppress=62` (negative shift count) each
  abort as above.
- `--diag_suppress=61` aborts for a shift count or pointer offset that does not fit the host integer
  (`1 << (unsigned __int128)-1`).
- Without suppression, and with `--diag_suppress=28`, there is no internal error.

## Fix

Branch `0022`, one commit on upstream `main` `ec2f12cffa`: "Leave a suppressed note out of its error instead of
asserting."
- **The change.** `add_more_info_list`, the only place a list of notes is attached to a diagnostic, frees the entries
  whose severity is `es_none` instead of attaching them. A suppressed diagnostic is not displayed. The error is still
  reported, without that note, and the notes of numbers that are not suppressed are shown as before. In the SARIF
  output the suppressed note is no longer listed as a related location (it did not fail there).
- **Not covered by the test:** the pragma form, the SARIF output, and an error with several notes of which one is
  dropped (the Opus review reproduced the pragma and SARIF behaviour by hand).
- **The test.** `changes/GH-diag-suppress-note.sft.cpp` (`--c++20`, five option sets: #39, #62 and #63 together,
  each alone, and none). It covers `constexpr` variables, enumerators, an array bound, a `static_assert` and a
  template argument, plus run-time uses of the same operators, whose warnings are suppressed as usual.
  - Fail-before: the upstream binary aborts on sets 1–4 (`ABORT--OUTPUT MISMATCH`) in `edg_x86_64` and
    `edg_x86_64_cp`, run natively on macOS arm64 (not in Docker, not the 32-bit configurations).
  - Pass-after: no mismatch.
- **Regressions.** Native `edgy -R` of `changes`, `regressions`, `cwg` and `edg` give the same mismatching tests
  before and after: native macOS platform differences, none new.
- **Still to run** (it is merged and pinned): the full suites on the x86-64 box, and the Changes check in Docker with
  an issue number.
- **A related display bug, not fixed by this commit.** With `--diag_remark`, `--diag_warning` or `--diag_error` on
  a note's number, the note is no longer `es_more_info`, so `construct_text_message` writes it as a diagnostic of its
  own and the whole error is displayed twice (under a "1 error detected" summary). Not applying the severity override
  to notes would fix both, but it would make `--diag_suppress` ineffective for notes and change existing output; the
  commit message explains the choice.

## Impact on cerastes

`cerastes verify` suppresses #39, #61, #62 and #63, because cerastes checks those operations itself (decision 0010
item 12). So in C++ an out-of-range constant expression (`enum { A = 1 << 32 };`, `constexpr int z = 1 << 32;`) stops
with this internal error, and `cerastes verify` exited 3 (a tool failure, never a verdict) instead of refusing the
program with error #28 (exit 2). Making the numbers remarks instead would stop the failure but print the error twice
(the related display bug above), which the test harness rejects. Decision 0019 item 9 kept the suppression list and
waited for this fix; since the pin `f0ed5525af` the shapes give error #28 alone, exit 2.
