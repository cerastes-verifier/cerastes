# S6 — A tutorial and a developer guide (docs session)

## Context

The user docs (`doc/`, https://cerastes.dev/cerastes/doc/) are reference pages: about 4,600 lines written like a
spec, with no learning path. The owner wants a VerCors-style tutorial and a developer guide. We agreed (2026-11-04)
that both live in this repo's `doc/`, not the website repo. That way every tutorial example stays a test run by
`check.sh` (invariant 8), and the tutorial changes in the same commit as the behaviour it teaches. The website becomes
the front door that links to them. The owner's answers at planning:

- **Question 131:** this session runs now, before the M4.5d2a re-audit's B1/S1 fixes. It touches no encoder code.
- **Question 132:** a minimal landing page in `cerastes-verifier.github.io`.
- **Question 133:** a tutorial chapter lands with the session that completes a concept.

`overview.adoc` is also stale. It says pointer parameters are rejected, says there are "sixteen kinds of error", and
limits pointers to "inside one function". This session fixes it.

**Reader-facing layout:** sections of the `ROOT` module, not new Antora modules.
`check_doc_examples.py` deliberately refuses any module other than `ROOT` (`check_modules`, line 1145; decision 0004
item 7). Subdirectories of `pages/` are already linted (`pages/**/*.adoc`) and give the same URLs:
`…/doc/tutorial/first-proof.html` and `…/doc/dev/architecture.html`. One `nav.adoc` holds three titled lists:
*Tutorial*, *Reference* (today's pages) and *Developer guide*. The lint reads only `nav.adoc`.

## Setup

- Branch `s6-tutorial` in the worktree `.claude/worktrees/s6`, created from `origin/main` (`bb41d55`, same as local
  `main`). Submodules are cloned with `--reference`, then `scripts/bootstrap.sh` runs in the background while pages are
  drafted (handoff §6).
- First step: save this plan as `.claude/plans/s6-tutorial.md` and commit it as the planning commit.
- Nothing under `src/`, `include/` or `tests/` changes. A doc example that exposes a bug is recorded in handoff §3 and
  not fixed here.

## Tasks

1. **Record `internal/decisions/0049-tutorial-and-developer-guide.md`**, written first. It amends 0004:
   1. Sections in `ROOT`, not modules (0004 item 7's lint unchanged).
   2. The nav's three titled lists. The start page stays `overview.adoc`, as a landing page with a "Where to start"
      section.
   3. The tutorial's form. Chapters are read in order. Each is a verified example, a failing variant with its error,
      and the fix. Examples are `.c` files verified in both modes, and positives are `native-ok`. `console` blocks are
      copied from real output at the session's tip and checked by review: the harness checks the category and line,
      not message text.
   4. The upkeep rule (question 133). Arrays come with M4.5e1, predicates and lists with M4.5f2, strings with M4.6,
      function pointers with M4.7, templates with M5, concepts with M6, classes and references with M7. Every session
      keeps existing chapters true.
   5. The developer guide summarises and links the canonical sources (`CLAUDE.md`, `tests/README.md`, `internal/`). It
      never becomes a second copy of their rules.
   6. The website carries links, no code, because nothing there is tested.
   7. 0004 item 8 ("pages describe only what exists") applies to both new sections.
2. **Harness lists.** Add `doc/modules/ROOT/examples/tutorial` and `…/examples/dev` to `NATIVE_POLICY_DIRS` and
   `DUAL_MODE_DIRS` in `scripts/run_tests.py` (lines 344 and 655). Update their pins in `scripts/test_run_tests.py`
   (`test_the_policy_directories_are_pinned` line 6650, `test_the_directories_are_pinned` line 12840). No other tooling
   changes.
3. **Tutorial** in `doc/modules/ROOT/pages/tutorial/`, with examples in `doc/modules/ROOT/examples/tutorial/`. Each
   shown block is one tagged test, written in the style of `examples/error-messages/*_{error,fixed}.c`: `CERASTES_*`
   macros, `// tag::source[]`, `// mode: c23, c++20`. About 8 pages and 40 to 50 example files:
   1. `index.adoc` — what deductive verification is, the write → verify → read the error → fix cycle, prerequisites
      (Installation), and how the examples are tested.
   2. `first-proof.adoc` — `ensures`, running `cerastes verify`, a `postcondition` failure and how to read it,
      `requires` and a `precondition` at the call, `CERASTES_CHECK`.
   3. `integers.adoc` — nothing wraps: an averaging `overflow` fixed by a precondition or by rewriting, unsigned
      arithmetic, `conversion`, `division-by-zero`.
   4. `loops.adoc` — why invariants are needed, `invariant-entry` and `invariant-preserved`, `decreases` and
      `termination`, recursion with a measure.
   5. `calls.adoc` — modular reasoning (a caller sees only the contract), a postcondition strengthened so that a
      caller verifies, trusted declarations and the single-unit assumption, `noreturn`.
   6. `pointers.adoc` — `swap` with `owns` and `old`, `permission`, `reads` against `owns`, `null-dereference`,
      `lifetime`.
   7. `structures.adoc` — a `struct` updated through `p->x`, contracts over its members, copies.
   8. `heap.adoc` — `malloc`'s NULL, `free`, `leak`, `invalid-free`, `owns_block` on a result. No loop over a heap
      place aliased by another pointer (B1's shape). Stop rule: an example that exits 3 or times out is replaced, not
      pinned.

   Each chapter ends with "Where to read more", linking the reference sections. The test-mutator subagent writes the
   "break it" variants from the positives and the intended category of each chapter. I write the prose.
4. **Developer guide** in `pages/dev/`, with examples in `examples/dev/` (the test-format examples). This is a complete
   first version, not a skeleton:
   1. `index.adoc` — the audience, repository layout, canonical sources, and how work is organised (milestones,
      sessions, decision records).
   2. `architecture.adoc` — EDG in the cerastes configuration (0002), the in-process back end and IL walk, VIR, Viper
      text and the position map, the driver (Silicon, Z3, error mapping), vlibc.
   3. `building.adoc` — `bootstrap.sh`, the pins (0001), `build/`, `env.sh`, EDG built twice, `CERASTES_EDG_SRC`
      (0005), Linux.
   4. `testing.adoc` — `expect:` headers (links `tests/README.md` for the grammar), twins, goldens, `run_tests.py`,
      `mutate.py`, `lint_viper.py`, `check.sh`, CI tiers (0016).
   5. `soundness.adoc` — reject, never approximate. Covers `inhale-ok`, tagged axioms, trusted contracts (vlibc's
      tags), audits, `internal/soundness/`.
   6. `contributing.adoc` — the life of an encoding rule: decision record → must-verify and must-fail tests → encoder
      → audit → reference section and tutorial chapter. Never edit an `expect:` line.
   7. `edg.adoc` — the fork's `cerastes` branch, upstream merges, bug-report format. Upstream reports are filed by the
      maintainer.
   8. `documentation.adoc` — the Antora layout, examples are tests, the two lint passes, handoff §6's doc pitfalls.
5. **Reference touch-ups.** Rewrite `overview.adoc`: drop the stale status and counts, give a short accurate summary
   linking each reference page, and add "Where to start". In `installation.adoc`, "Verify your first program" links
   the tutorial. In `nav.adoc`, the three lists.
6. **Website** (`../cerastes-verifier.github.io/src/pages/index.astro`): replace "Under construction" with one
   paragraph and links to Tutorial, Reference and Developer guide. Keep the brand styles, add no code. Committed to
   that repo's local `main`; the owner pushes.
7. **Project files.** In `CLAUDE.md`, the User docs line names the three sections. In `plan.md`, an S6 row in Done,
   the upkeep rule in M4's **Docs** paragraph, and a tutorial line in the Docs of M5, M6 and M7. Also update
   `internal/status.md` (entry and first step: B1/S1) and `internal/handoff.md` (§1, §3, questions 131 to 133 in §8).

## Agents

The owner approved three Sonnet implementers, each with its own files. Only single-file Silicon runs happen while
agents work; I run the directory-wide runs.
- **examples** (general-purpose, Sonnet): the positive tutorial examples for chapters 2 to 8 from the outline above,
  each verified in both modes and compiled natively.
- **test-mutator** (Sonnet): task 2's harness lists and self-test pins, then the "break it" variant of each positive,
  each failing for the intended category at the intended line.
- **dev-guide** (general-purpose, Sonnet): drafts `pages/dev/*.adoc` and `examples/dev/` from the canonical sources.
  I review and edit them.

I write record 0049, the tutorial prose, the overview, nav and website changes, and the project files, and integrate
everything.

## Review

- The soundness-auditor reviews the harness diff (task 2) and runs a claims audit of every new and changed page.
  Each statement about behaviour, trust or a result's meaning is checked against the code, the reference pages and
  the real output. One round, plus a re-audit of blocking fixes.
- I look at the built site in a browser (nav, the three sections, page links) before the final check.

## Verification

```sh
python3 scripts/run_tests.py doc/modules/ROOT/examples/tutorial doc/modules/ROOT/examples/dev
python3 scripts/check_doc_examples.py
scripts/build_docs.sh && python3 scripts/check_doc_examples.py --html build/doc
python3 scripts/test_run_tests.py
scripts/check.sh                       # full check, CERASTES_CHECK_BASE unset; at most twice
(cd ../cerastes-verifier.github.io && npm ci && npm run build)
```

## Commits

All on `s6-tutorial`, with no Claude trailer and no push:
1. planning
2. record 0049
3. harness lists
4. tutorial
5. developer guide
6. overview and nav
7. project files

Plus one commit in the website repo. The owner pushes both and opens the PR.
