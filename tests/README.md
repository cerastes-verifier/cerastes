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

| Directory | What it checks | Runner |
|---|---|---|
| `viper-smoke/` | Silicon and Z3 work, and the harness reads Silicon's errors correctly | Silicon |
| `edg-smoke/` | The pinned EDG front end parses C++20 and C23 against the system headers | `scripts/cpfe.sh` |
| `il/` | Golden tests of the IL walker (`--cerastes_walk`): the Gate G1 probes (`g1/`), annotation-free copies of the VeriFast tests (`verifast-plain/`), contrast probes that expose a vacuous pass (`contrast/`) and walker smoke tests | `scripts/cerastes-fe.sh` |
| `differential/verifast/` | Nothing yet: unmodified reference copies of VeriFast's template tests, with no `expect:` headers. Skipped (`.harness-skip`) | none |
| `../doc/modules/ROOT/examples/` | The code examples of the user documentation, which are ordinary tests (not under `tests/`, but run by default with it) | by the example's `expect:` kind |

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

**2. The built-site check**, `python3 scripts/check_doc_examples.py --html
build/doc`, run after `scripts/build_docs.sh` (in `check.sh` and in the CI
build job). It does not read the pages: it classifies every `<pre>` of the
generated HTML (and every `<code>` with a `language-X`, `lang-X` or `data-lang`):

- a block in a cerastes language must, after unescaping, equal a tagged region
  of a test under `examples/` (the lines of the chosen tags in file order, tag
  lines dropped; whitespace at line ends, common indentation and blank lines
  around the code are ignored), and the site must have at least as many such
  blocks as the page sources declare (so a UI change that renames the class
  cannot make the check pass without looking at anything);
- a block in `sh`, `console` or `text` is fine;
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

`check.sh` runs the examples as tests, both checks, and the docs build; CI
(the docs workflow) runs the source lint, `scripts/build_docs.sh` (which fails
on any Antora warning) and the `--html` check, but not the examples as tests,
because those need the EDG and Silicon toolchain. The harness self-test has a
twin for each rule; it checks that `check.sh` and the workflow run the checks
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

Categories for C and C++ tests are defined by the driver's error messages
(M3) and will be added here then.
