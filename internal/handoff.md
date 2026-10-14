# Handoff after M1.5 (2026-10-14)

The next session reads this file after `CLAUDE.md`, its own milestone in
`plan.md`, `internal/status.md` and `internal/decisions/`. `status.md` has the formal
M1.5 entry. This file adds what a cold start would otherwise have to
rediscover. The M1 material that is still open (§3 and §6) is carried over
from the M1 handoff unchanged.

## 1. State in one paragraph

Gate G1 passed in M1. M1.5 added the user documentation:
- an Antora component in `doc/`, published from this repository's GitHub
  Pages at **https://cerastes.dev/cerastes/doc/**. The org convention is
  `cerastes.dev/<repo>/doc/` (decision 0004);
- two pages, Overview and Installation, describing only what exists;
- a branded UI (vendored default UI plus supplemental files);
- lunr search;
- a CI workflow that lints, builds and deploys.

Every C/C++/Viper code block on a page is a tested example. Two checks
enforce it:
- a source lint;
- a rendered-HTML backstop, which requires every code block to equal a tagged
  region of a test.

`check.sh` builds the docs and fails on any Antora warning. It is green in
about 32 s (305 self-tests, 45 tests).

Developer docs moved from the `docs` directory to `internal` at the end of
M1.5 (owner: two folders named `doc` and `docs` were confusing).

## 2. M1.5: things to look at again

1. **The rendered-HTML check is the real guarantee.**
   - `scripts/check_doc_examples.py` reads AsciiDoc line by line. It is
     best-effort, not a parser. Its known gaps are in `tests/README.md`:
     nested tables, an indented paragraph in a list continuation, and a
     `++++` pass block with raw HTML.
   - `--html build/doc` catches any of those in the built site.
   - Neither check can tell that a block shows a region of the *wrong*
     example file, only that it equals some tested region.
   - One include per code block (`tags=a;b` to combine).
2. **Antora path resolution.**
   - In a playbook, `./`-prefixed paths resolve against the playbook's
     directory; everything else, including `..`, resolves against the current
     directory. `scripts/build_docs.sh` therefore changes into the playbook's
     directory first.
   - It forces the checksummed UI bundle with `--ui-bundle-url`, so no
     playbook can substitute another.
3. **`site.url` matters for `404.html`.**
   - Ordinary pages use relative links. `404.html` uses root-absolute
     `/cerastes/doc/_/…` links, which come from `site.url`.
   - The workflow copies `404.html` to the root of the Pages artifact, the
     only place GitHub serves a custom 404 page from, and writes a root
     `index.html` that redirects to `doc/`.
   - If the address changes, change `site.url` (`doc/local-playbook.yml`), the
     staging step's canonical link in `.github/workflows/docs.yml`, and the URL
     lines in `CLAUDE.md` and `README.md`.
4. **The npm stamp checks metadata only.** `bootstrap.sh` compares
   `doc/node_modules/.package-lock.json` with the lockfile (versions,
   resolved URLs, integrity) and hashes `package.json`. Files edited in place
   inside `node_modules` are not detected. Any edit to `doc/package.json`,
   even its description, makes `check.sh` stop with "run
   scripts/bootstrap.sh" (the rename to `internal` did exactly that).
5. **`--build-only` is not offline.** It skips Homebrew, the Z3 download and
   `npm ci`. It still initializes a missing submodule and rebuilds Silicon on
   a stale stamp, which can make sbt fetch dependencies. This is deliberate:
   `check.sh` must rebuild after a pin move. The help text and the
   Installation page say so.
6. **Node is pinned by major only** (`doc/.node-version` = 26, read by CI).
   Bootstrap installs Homebrew's floating `node`. Versioned `node@NN`
   formulae exist if the floating major ever breaks Antora.
7. **`pass:[C++]`.** In AsciiDoc prose a bare `C++` opens a passthrough that
   silently swallows text, and Antora does not warn. The owner's rule is to
   write `pass:[C++]`, and the lint enforces it (rule e).
8. **The headers EDG cannot parse have three causes, not one.** The
   correction is in 0001:
   - `_stdlib.h` block syntax, which also breaks `<utility>`;
   - `__algorithm/find.h`;
   - `__new/interference_size.h`.

   M0 had listed `<utility>` as clean.
9. **The docs-only link colour.** Sand fails contrast as text on white, and
   Sand Deep is 2.9:1. Links therefore use a new token, `--cerastes-sand-ink`
   `#7f5f2d` (5.9:1), in `doc/ui/supplemental/css/cerastes.css`. It is not in
   the brand kit (question 3).

## 3. Things to look at again (from M1, still open)

These held up for M1 but are compromises or open questions.

1. **Generated EDG headers come from the stock build.** `cmake_defines.h`,
   `err_codes.h` and `err_data.h` are read from `build/edg/src/includes`
   (0002 §3). That `cmake_defines.h` silently pre-empts `host_envir.h`'s
   derived defaults: it hard-codes `DEFAULT_INSTANTIATION_MODE tim_none` and
   `DEFAULT_REMOVE_UNNEEDED_ENTITIES 0`. Everything load-bearing is now
   pinned in `edg_config`, checked by `static_assert`s in `src/backend/edg_il.h` and
   checked again by `scripts/check_edg_config.sh`.
   - A new load-bearing macro must be added in all three places.
   - If the stock base configuration ever changes, consider generating the
     headers ourselves (`edg-cmakedef`, `mk_errinfo`).
2. **Shared-expression budget instead of a visited set.** EDG's `Ptr_set`
   allocates from the front-end region, which is dead once `back_end()`
   runs. So the walker caps the number of recorded expressions it follows (1,000,000) instead of
   tracking visited shared expressions, and prints a truncation warning if
   it is reached. It is never reached in the tests.
   - The walker reports each operation node at most once per routine, and
     each use of a template-parameter constant (the pair of constant and
     `enk_constant` node) at most once. It keeps its own per-routine sets,
     because a node can be reached both through a constant's backing
     expression and through the `ck_template_param` variant's expression.
     The dedupe hides double traversal. Audit pass 3c found duplicate
     `tconst` lines only by counting constants against `il_display`, so
     count, don't just diff, when the walker changes.
   - M3/M4 encoders that walk expressions need their own visited set
     (`std::unordered_set` in our code, not EDG's allocator).
3. **Lambda names come from `output_name`/`output_class_qualifier` hooks.**
   This replaces EDG's `lambda [] type at line <seq>…` (which prints a
   sequence number labelled as a line) with `<lambda@L:C>`. For every other
   entity the hook defers to EDG's own `form_type`/`form_name`. An earlier
   version that re-implemented EDG's tag-keyword logic broke C `enum` tags and
   the template arguments of unnamed types. If type rendering ever looks off,
   look here first (`src/backend/render.cpp`) and compare with `il_display`.
   Known limit: a closure that appears as a template argument inside another
   name keeps EDG's default spelling.
4. **Primary-file filter.** Only routines whose position is in the primary
   source file are reported. Code in user headers or after `#line` is hidden
   but counted: `end walk: … <U> routines with bodies in non-system files
   outside the primary file`, plus a warning for each `cerastes::` attribute
   there. A `#line` or GNU linemarker in the primary file gets its own
   warning. Code after a flag-3 ("system header") linemarker is neither
   reported nor counted, so that warning is its only trace. M3's driver needs
   a real policy for user headers, because verified code will live in them.
   M3 must also own the command line: `cerastes-fe` passes every EDG option
   through. It rejects `--remove_unneeded_entities` and its abbreviations, by
   checking EDG's runtime globals in `back_end()`, but options such as
   `--instantiate=none` would still change the IL.
5. **The goldens are tied to the toolchain.** Constraint lines print libc++'s
   inline namespace (`std::__1::integral`), and `il_to_str` has no option to
   hide it. Goldens also carry EDG's #2803 warnings in their stderr section.
   - Expect mass golden diffs when the macOS SDK or the EDG pin moves, and
     when M2 registers the `cerastes` namespace (the #2803 lines disappear).
   - Each regeneration needs a reviewed reason (invariant 1; `tests/README.md`).
6. **Facts `il_display` cannot show.** It never prints attribute entries that
   hang off statements, nor some constant entries (`tpck_unknown_function`
   callees, `tpck_member`). For those the walker is the only reader. The
   inventory records the differential method used instead (mutate the source,
   watch only the expected walker lines change). Do not conclude that
   something is missing from the IL by grepping `il_display`: one agent did
   that this session and reported a false loss of null-statement attributes.
7. **Default arguments EDG has not evaluated** have `default_arg_expr == NULL`
   and are not walked (noted in the walker's contract comment).
   **Folded constants.** EDG folds non-dependent constant expressions. The
   operations survive only as the folded constant's recorded expression
   (`constant->expr`). The walker follows two kinds of recorded expression:
   - those of `ck_template_param` constants;
   - those of unnamed constants whose expression lives in the routine's own
     memory region (`!in_file_scope(c->expr)`), i.e. folds written in the
     body, such as `3 + 4`, `1 << 4` and `sizeof(T) * 2`.

   It does not follow named entities' values, such as an enumerator's
   initializer, or file-scope expressions, such as another function's
   default argument used at a call site. Both would otherwise be printed as
   if they were in the routine. M4's eligibility checker needs the same
   distinction. Expressions embedded in types (array bounds, C VLA bounds,
   `Holder<sizeof(T) / 2>`) and `static_assert` expressions are not walked.
   A routine's own default arguments are walked with file-scope folds
   allowed. That currently also pulls in a callee's default argument nested
   inside one (`c = h() + 1`), which shows at the callee's position. The fix
   is in inventory "Walker gaps"; no golden exercises it. For M3 this means a
   non-overflowing constant expression such as `INT_MAX - 1 + 1` may reach
   the encoder already folded. An overflowing one (`2147483647 + 1`) keeps
   its `eok_add`, with a warning and exit status 0. `6 / 0` is only a warning.
   M3's integer-semantics decision record has to say which overflow checks
   happen on folded constants, and how.
8. **No helper headers next to tests.** Discovery treats every source-like
   file (`.h`, `.hpp`, `.cc`, …) under `tests/` as an error, so that a
   mis-suffixed test is never silently skipped. M3 and M6 tests that need a
   `#include "helper.h"` will need an escape hatch, for example an allowlisted
   `include/` directory. Add it with a self-test twin, as for the skip
   allowlist.
9. **Rendering of `type`.** A dependent operation's type is the
   `tptk_unknown` placeholder and prints as `<tptk_unknown>`. EDG itself calls
   it `<unnamed>`.

## 4. Friction (process and tooling)

- **The team, with the agent types loaded from session start.** M1's friction
  is solved: `edg-backend`, `il-analyst`, `test-mutator` (Sonnet) and
  `soundness-auditor` (Opus) all ran as real types. Ownership was:
  - `edg-backend`: build, CI and UI;
  - `test-mutator`: harness and lint;
  - `il-analyst`: pages, then the upstream draft.

  File ownership was disjoint, and resuming agents with `SendMessage` kept
  their context through two rounds of fixes.
- **The audit paid for itself again.** Pass 1 found two blocking holes in
  "every example is a test" that the self-tests had enshrined:
  - partials and non-test files could be included;
  - table-cell, extra-module and component-wide `source-language` forms
    rendered as C++ but were never linted.

  The fix that holds is the rendered-HTML backstop, not more parser rules.
  Keep the order: implement → independent check → audit.
- **Agents left stray files in the tree.**
  - Running stock `cpfe` without `--no_code_gen` wrote `hello.int.c` into the
    repository root.
  - `sed -i -E` on macOS left `*.adoc-E` backups.

  Both were caught and removed. Tell agents to run probes only in the
  scratchpad, and check `git status --porcelain --untracked-files=all`
  before each commit.
- **An agent rewrote a file from its own copy** and dropped a line the
  orchestrator had added (`docs.yml`). Don't edit agent-owned files while the
  agent is active. When the orchestrator must, tell the agent to edit in
  place.
- **Global replaces need review.** A blanket `cerastes.dev/doc/` →
  `cerastes.dev/cerastes/doc/` replace also rewrote the superseded plan text
  where the old address was correct. It was caught and reverted.
- **cerastes.dev had no TLS certificate for the custom domain** on
  2026-10-13. `www` was proxied by Cloudflare, which blocks GitHub's
  Let's Encrypt issuance, and `.dev` is HSTS-preloaded, so browsers could not
  open the site at all. The owner fixed the DNS and enabled "Enforce HTTPS"
  on 2026-10-14.
- **Headless Chrome works for screenshots.** Serve over `python3 -m
  http.server` (`file://` breaks root-absolute paths), and run Chrome with
  `--headless=new --screenshot` and a scratchpad `--user-data-dir`.

## 5. For M2 (annotation layer)

Updated for M1.5. The upstream proposal is drafted and its scope was set by
the owner.

- **The upstream Discussion is drafted:**
  `internal/upstream/attribute-registration.md`, with 16 probes in
  `internal/upstream/attribute-registration-probes/`. Re-run any of them with
  `scripts/cerastes-fe.sh --c++20 --cerastes_walk <probe>`.
  - **Scope (owner, 2026-10-14).** cerastes aims for all of VeriFast's
    functionality and more. Specifications are **C++ grammar inside
    `[[cerastes::…]]`**, using spec functions from `<cerastes.h>`
    (`owns`, `old`, `forall`…), never VeriFast's `|->`/`&*&`/`?x`. The owner's
    canonical example is `bump()` in §3 of the draft. EDG is asked for no new
    tokens, operators or expression grammar.
  - **Requirements R1–R8.** Registration; placement on every entity and
    statement; redeclaration merging; typed arguments; scopes and binders
    (result binder); template-aware IL; spec-only entities (ghost fields
    need a layout exclusion); diagnostics.
  - **The design** is phased in five steps. M2's eight attributes are the
    first use, not the scope.
  - **Appendix A** inventories VeriFast's annotation language (about 45
    features), each with one possible C++ form. The forms are illustrative
    unless plan.md fixes them.
  - **Open cerastes design points** (not EDG's): pattern binders (`?x`),
    predicate input/output parameters, inductive types with `switch`,
    fractions, lemmas, ghost assignment, function-pointer chunks.
    Concurrency and shared boxes are outside plan.md. M7's inventory owns the
    full list.
  - **The owner files it** (decision 0003 item 6). Sessions never open
    upstream issues, PRs or Discussions.
- **Measured facts M2 builds on** (from the probes; draft §5):
  - **After the parameter list, the IL keeps at most one declaration's
    attribute, on the function type, not the routine.** For non-templates and
    out-of-class members it keeps the defining declaration's; if the
    definition has none, none survives. For templates it keeps the first
    declaration's. (This refines inventory check 5c.)
  - Prefix attributes and GNU-style postfix attributes accumulate across
    redeclarations.
  - Expression-signature attributes (`enable_if`, `assume`) are parsed and
    type-checked at the attribute's line. `this` and lambdas work. Prototypes
    keep dependent expressions, and instances get substituted ones.
  - Attributes are recorded on 17 entity kinds. They are rejected on
    template parameters, around the word `concept`, and inside
    requires-expressions; that matters for M5.
  - An attributed field still takes storage.
- **The fork branch is still not fast-forwarded** (question 2). It is at
  `90b66472`, 22 commits behind the pin `9847356d`. Upstream `main` is now
  `034385ba`, 8 commits past the pin; none of them touches `src/`.
  Sessions commit in a local clone of the fork (`~/Documents/compiler`), and
  **the owner pushes** (0003 item 6, answer B).
- **cerastes code cannot depend on the registration until it is upstream**
  (0003 item 8). Prefer the general mechanism in the draft over hard-coding
  `cerastes` into `attribute.c`.
- **Ill-formed placements become silent once the namespace is registered.**
  `[[cerastes::x]] struct S {};` (a leading attribute with no declarator)
  appertains to nothing and is not in the IL. Today only #2803 betrays it;
  after registration, nothing will. Give it a diagnostic (draft R8) or a
  must-fail test.
- **What the unrecognized path does carry reliably:**
  - prefix attributes on function declarations (plain, function template
    prototype and instance, class-template members);
  - attributes on `while`/`for`/`if`/`switch`/`do`/compound/expression
    statements;
  - attributes on null statements in every position tried.

  So `check(p)` on a null statement and `invariant(p)` on loops are safe
  placements.
- **Class-level annotations go after the class-key** (`struct [[x]] S`).
- **Golden tests.** Compare walker output, not `il_display`, which prints
  addresses and omits statement attributes. Extend `--cerastes_walk` to print
  the typed expression tree of each `cerastes::` attribute argument.
- **M2's user docs.** The annotation reference (`doc/…/annotations.adoc`)
  and the page on building with ordinary compilers through `<cerastes.h>`.
  - Every C/C++ block is an `include::example$<test>[tag=…]` of a test under
    `doc/modules/ROOT/examples/`. Golden tests work there too: the `.golden`
    goes next to the example.
  - Write C++ in prose as `pass:[C++]`.
  - Both lints run in `check.sh` and in CI.

## 6. For M4 (generic templates; unchanged from M1)

From the inventory's gotchas; each is evidenced there.

- **`(T)<literal>` is not an `eok_cast`.** It is an `enk_constant` of
  `ck_template_param`/`tpck_expression` whose `expr` is an `eok_cast` with
  `type_kind tk_unknown`.
  - A naive fail-closed rule ("reject `tk_unknown` operations") would reject
    `clamp100`'s `(T)100` and `two_hundred`'s `(T)200`, which VeriFast treats
    as literal casts.
  - The eligibility checker must recognise this exact shape (cast of an
    integer literal to a template parameter) and still fail closed on every
    other `tk_unknown`.
- **`type_kind` is not a dependence test.**
  - Most dependent operations are `tk_template_param`.
  - `eok_question`, `eok_call` and casts inside constants are `tk_unknown`.
  - `eok_land`/`eok_lor` are `tk_integer` even in prototypes.

  Dependence needs a recursive type walk; there is no `isDependentType`
  flag.
- **Expressions hide inside `tpck_expression` constants:** `!(a<b)`, default
  arguments, `&r`. Walk `constant.expr`, as the walker now does.
- **Telling prototypes from instances.**
  - Both carry `assoc_template` and `template_arg_list`; the prototype's
    list holds the template's own parameters.
  - An instance is a routine *without* `is_prototype_instantiation`.
  - There is no per-template list of instances, so scan all routines.
  - `template_arg_list` is NULL for members of class-template instances.
- **Constraints.**
  - `std::integral` is `std::__1::integral`, so match the concept template
    entity, not a string.
  - For a type-constraint, `concept_id.args` is NULL; the constrained
    parameter is implicit.
  - The four locations are recorded in inventory check 4.
- **VeriFast originals and parity.** The originals are in
  `tests/differential/verifast/` (skipped by the harness; README gives the
  source commit and the `testsuite.mysh` lines). Annotation-free copies with
  goldens are in `tests/il/verifast-plain/` (line N there is line N−6 of the
  original). VeriFast still has to be built or recorded (0001 §VeriFast); it
  is a Linux binary here.

## 7. M1.5 acceptance (commands and results, 2026-10-14)

| # | Command | Result |
|---|---|---|
| 1 | `scripts/bootstrap.sh`, twice | the first run does `npm ci (doc/)`; the second prints `doc dependencies are current`. With `doc/node_modules` moved aside, or its `.package-lock.json` tampered with (4 ways), `--build-only` fails: `doc/node_modules is missing or stale; run scripts/bootstrap.sh` (edg-backend) |
| 2 | `scripts/build_docs.sh` | exit 0, zero warnings; `build/doc/` has `index.html`, `overview.html`, `installation.html`, `404.html`, `search-index.js` and `_/` with the brand assets. Lunr search tested in headless Chrome ("pipeline": 2 results), including under a `/cerastes/doc/` prefix |
| 3 | `python3 scripts/run_tests.py` | `PASS doc/modules/ROOT/examples/installation/hello.cpp [c++20]`; 45 passed, 0 failed, 1 skipped |
| 4 | `python3 scripts/check_doc_examples.py`; `… --html build/doc`; `CERASTES_REQUIRE_E2E=1 python3 scripts/test_run_tests.py` | OK; OK (4 html files, 1 code block); 305 tests OK, 0 skipped. The e2e twins build real Antora sites: warnings fail `build_docs.sh` but not bare Antora; the lint-evading pages q1–q18 and e1/e2/e5 fail |
| 5 | `scripts/check.sh` | green in 31.5 s; `git status` shows no stray files |
| 6 | shallow clean clone (`git clone --depth 1`) of the M1.5 state: `scripts/bootstrap.sh && scripts/check.sh` | pending: runs right after the M1.5 commit |
| 7 | branding | `internal/images/m1.5-docs-overview.png` (local build served under `/cerastes/doc/`) next to `internal/images/m1.5-website-reference.png`: Ink navbar with the lockup, Sand rules and markers, Archivo, favicon |
| 8 | soundness audit | pass 1: 2 blocking (examples could be included or rendered without being tested). Pass 2: 2 blocking (`<pre>` without a language; garbled `libc++` on the page). Pass 3: nothing blocking. Its three follow-ups (`subs=`, the tag-directive form, per-item counting) are fixed with twins and are covered by the green self-test, not re-audited |
| 9 | after the owner's push: the Actions run, `curl -sI https://cerastes.dev/cerastes/doc/`, live search | pending the owner's push |
| 10 | docs-only commit pushed by the owner | pending the owner's push |
| 11 | `git -C third_party/edg status --porcelain`; `git submodule status` | empty; EDG still at `9847356d`, Silicon at `6b540aa4` |

## 8. Questions for the owner

Every open decision from this session, each with a recommended answer. Answer
them inline, as last time.

1. **File the upstream Discussion?** The draft is
   `internal/upstream/attribute-registration.md`.
   - §1–§9 are the post; Appendix A is the VeriFast inventory; "For the owner"
     is not part of it.
   - It follows your two scope corrections: VeriFast parity and beyond, and
     C++ grammar inside the attributes, with `bump()` as the lead example.
   - The auditor reproduced its probes.

   *Recommended: read §1–§6, then file it as a Discussion
   (`CONTRIBUTING.md` says feature requests start there). Keep Appendix A in
   the post, so the breadth is visible.*
2. **Fork branch and pin.** The fork's `cerastes` branch is still at
   `90b66472`, 22 commits behind our pin `9847356d`. Upstream `main` is
   `034385ba`, 8 commits past the pin: CI, tests and `doc/source` only,
   nothing under `src/`.

   *Recommended: at the start of M2, move the pin to the upstream head
   (`check.sh` re-checks every golden), then fast-forward the fork branch to
   the same commit:*
   ```
   cd ~/Documents/compiler && git fetch edgcpp && git checkout cerastes
   git merge --ff-only <new pin> && git push origin cerastes
   ```
3. **Link colour.** On white, Sand fails contrast for text (2.2:1), and Sand
   Deep is 2.9:1. The docs use a new token, Sand Ink `#7f5f2d` (5.9:1).
   *Recommended: accept it, and add it to BRAND.md as the text colour for
   Sand on light grounds.*
4. **Which lockup is canonical?** The loose
   `brand/cerastes-lockup-on-dark.svg` differs from the copy in the kit zip
   and on the website; the docs use the kit/website copy. *Recommended: the
   kit's. Replace the loose file.*
5. **The "Documentation" link on cerastes.dev** (website repo, your step).
   *Recommended: link to https://cerastes.dev/cerastes/doc/.*
6. **The `github-pages` environment.** Please confirm that Settings →
   Environments → `github-pages` allows deployments only from `main`. The
   workflow already deploys only from `main`, but the environment rule is the
   second lock, and sessions cannot see it.
7. **CI for the rest of the repository.** You asked for it soon.
   *Recommended: a short session before M3 writes a decision record. Try a
   macOS runner with a pinned Xcode first; if the IL goldens drift, use x64
   Docker as you suggested. Then add `check.sh` to CI.*
8. **Dependabot for the pinned Actions?** The workflow pins four actions by
   commit SHA. Dependabot (as in the website repo) would open PRs to bump
   them. *Recommended: yes, `github-actions` ecosystem only, weekly. It opens
   PRs on GitHub, so it's your call.*
