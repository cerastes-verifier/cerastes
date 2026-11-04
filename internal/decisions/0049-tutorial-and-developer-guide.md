# 0049 — A tutorial and a developer guide: three sections of one documentation site

- **Status:** accepted (2026-11-04, S6), written before the pages (invariant 6). Owner decisions: the layout (S6
  planning, 2026-11-04: "I agree this is the right approach" to keeping the tutorial and the developer guide in this
  repository's `doc/`, with the website as the front door) and handoff questions 131 (this session runs before the
  M4.5d2a re-audit's fixes), 132 (a minimal landing page on the website) and 133 (a tutorial chapter lands with the
  session that completes its concept).
- **Milestone:** S6 (docs), and every later session that completes a concept the tutorial teaches (item 4).
- **Amends:** 0004 (the user documentation site): item 1's site gains two sections; items 7 and 8 apply to them
  unchanged.

## Context

The user documentation (`doc/`, https://cerastes.dev/cerastes/doc/) was eight reference pages: what each annotation
means, what `cerastes verify` checks, every error category, the integer and memory rules. They are written like a
specification and have no learning path: a reader who does not yet know what a contract or a loop invariant is has no
page to start from. VerCors, the reference of M4.5's pointer encoding, publishes a tutorial that builds the concepts one
chapter at a time, and the owner asked for one in that style, and for a developer guide.

Where they live was the question. The website (`cerastes-verifier.github.io`, Astro) is a single "under construction"
page. A tutorial there would sit outside the test harness: invariant 8 makes every code example in `doc/` a test that
`check.sh` runs, and a tutorial is mostly examples, about a tool whose accepted language changes every session
(pointer parameters went from rejected to verified in a week). The website's examples would rot unseen. Prose rots even
inside `doc/`: at S6's start `overview.adoc` still said that pointer parameters are rejected and that there were sixteen
error categories (there are twenty). Record 0004 item 2 also rejected building the docs in the website's workflow, to
keep the two tool chains apart.

`scripts/check_doc_examples.py` refuses any Antora module other than `ROOT` (0004 item 7, "a new module is a reviewed
change"), because its source lint reads `ROOT`'s pages only. It reads `pages/**/*.adoc` and `nav.adoc`, so
subdirectories of `pages/` are already linted.

## Decision

1. **One site, three sections, one module.** The tutorial is `doc/modules/ROOT/pages/tutorial/`, the developer guide
   `doc/modules/ROOT/pages/dev/`, and the existing pages stay where they are as the reference. Their examples are
   `examples/tutorial/` and `examples/dev/`. The URLs are `…/cerastes/doc/tutorial/<page>.html` and
   `…/cerastes/doc/dev/<page>.html`. No second Antora module: the lint's `ROOT`-only rule and everything 0004 item 7
   checks apply to the new pages without a change to the checker.
2. **Navigation.** `nav.adoc` holds three titled lists, *Tutorial*, *Reference* and *Developer guide*, in that order.
   The start page stays `overview.adoc`, which says what cerastes is and does today in a short summary that links the
   reference pages for the details (no counts of categories or constructs, which rot), and has a "Where to start"
   section: new users to the tutorial, readers looking up a rule to the reference, contributors to the developer guide.
3. **The tutorial's form.** Chapters are read in order; each introduces one concept with a program that verifies, then
   the same program with one change and the error it causes, the way to read that error, and the fix; it ends with
   "Where to read more", linking the reference sections. Its examples are `.c` files verified in both modes
   (`c23`, `c++20`), and the positives compile with ordinary compilers (`native-ok`): `examples/tutorial/` and
   `examples/dev/` join `DUAL_MODE_DIRS` and `NATIVE_POLICY_DIRS` in `scripts/run_tests.py`, pinned by the harness
   self-test. A failing variant is named `<positive>_<what>_error.c`. The harness checks each error's category and
   line, not the message text, so the `console` blocks that show the messages are copied from a real run at the commit
   that changes them and checked by review, as on the reference pages. Chapters at S6: a first proof, integers, loops,
   calls, pointers, structures, the heap, over what `cerastes verify` accepts today (no arrays, predicates or
   templates).
4. **Upkeep** (owner, question 133). The session that completes a concept the tutorial does not yet teach adds its
   chapter in the same session, beside its reference section (invariant 9): arrays and quantifiers with M4.5e1 (pointer
   arithmetic extends it at M4.5e2), predicates and a linked list with M4.5f2 (M4.5f1's non-recursive predicates may
   wait for it), strings with M4.6, function pointers with M4.7, generic templates with M5, concept requirement
   contracts with M6, references and classes with M7. Every session keeps the existing chapters true: their examples are
   tests, and a session that changes a message, a category or an accepted construct a chapter shows updates the
   chapter's prose and `console` blocks in the same commit. A planned chapter is not written before its feature exists
   (0004 item 8).
5. **The developer guide summarises and links; it is not a second copy.** It is for contributors and reviewers: the
   architecture, building, testing, soundness, adding a feature, the EDG fork, writing documentation. The rules stay
   where they are (`CLAUDE.md`'s invariants and commands, `tests/README.md`'s test grammar, the decision records,
   `internal/encoding.md`, `internal/il-inventory.md`); a page states such a rule in a sentence or two and links its
   source on GitHub. `internal/` stays unpublished: `status.md`, `handoff.md` and the session plans are working logs,
   not documentation. A page shows a test file's header or a directory tree in a `text` block quoting a named real file
   (reviewed), because a cerastes-language block must be a tagged region of a test (0004 item 7), and tagged regions
   never show the header.
6. **The website carries links, not code.** Its landing page says in one paragraph what cerastes is and links the
   tutorial, the reference and the developer guide. It shows no C or C++, because nothing in that repository
   is tested against the verifier. It is committed in its own repository and deployed by its own workflow (0004
   item 2).
7. **0004 item 8 applies to both sections**: they describe only what exists. A chapter or guide page may say what is
   not supported yet and link the reference lists, as the reference pages do.

## Consequences

- `scripts/run_tests.py`'s two policy lists and their pins in `scripts/test_run_tests.py` gain the two directories;
  `tests/README.md` lists them where it lists the documentation example directories.
- `CLAUDE.md`'s user-docs line names the three sections; `plan.md`'s M4 **Docs** paragraph gains item 4's rule, and the
  **Docs** lines of M5, M6 and M7 name their chapters.
- Every later planning names the tutorial chapter it owes, or says that it owes none.
- The `console` blocks are the part of a chapter no check enforces. A check that runs each shown command and compares
  its output is possible later work; S6 does not build it.
