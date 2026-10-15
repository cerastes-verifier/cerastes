# Handoff after M2 (2026-10-15)

The next session reads this file after `CLAUDE.md`, its own milestone in
`plan.md`, `internal/status.md` and `internal/decisions/`. `status.md` has
the formal M2 entry. This file adds what a cold start would otherwise have
to rediscover.

The M1.5 handoff (the docs site: Antora path resolution, `site.url` and
`404.html`, the npm stamp, `pass:[C++]`, the libc++ header causes) is still
valid. Read it with `git show c0b7ef2:internal/handoff.md`, §2.

## 1. State in one paragraph

M2 is done on `main`:
- EDG parses and type-checks the eight `[[cerastes::…]]` attributes as C++
  expressions;
- the walker prints each one, on the right entity, with a typed expression
  tree (dependent in template prototypes, substituted in instances);
- 24 cerastes-side checks reject what EDG cannot, failing closed on
  anything the listing cannot represent;
- `<cerastes.h>` makes the same files build with Apple clang and GCC under
  `-Wall -Wextra -Werror`;
- the Annotations page and the "Building with ordinary compilers" page
  document it, and every example is a test.

The EDG side is a local commit series on the owner's fork, branch
`cerastes`. **cerastes now pins the fork** (decision 0009, owner,
2026-10-14), so the series must be pushed before cerastes `main`. `check.sh`
is green, and its report line says `NOT YET PUSHED` until then.

## 2. Owner steps, in this order

1. **Push the fork:** `git -C ~/Documents/compiler push origin cerastes`.
   - GitHub's `cerastes` is still at `90b66472`, an ancestor of upstream
     `034385ba`, so this is a plain fast-forward to `23f55e3628` (K).
   - The local branch was moved to upstream `main` by the owner, then the
     series was added.
2. **Push cerastes `main`.**
   - After step 1, `scripts/edg_tree.py check` reports `(gitlink, clean)`
     without `NOT YET PUSHED`.
   - A clean clone fetches the submodule from the fork (`.gitmodules`).
   - Pushing also publishes the new docs pages (the docs workflow deploys
     on pushes to `main` that touch `doc/`).
3. **Push the website:** `git -C ~/Documents/cerastes/cerastes-verifier.github.io
   push origin main`. Commit `d313c84` adds the "documentation" link;
   sessions have no GitHub credentials.
4. **Run the full EDG `all` suite on the x86-64 box with a debug preset.**
   - The series' tests need `INCLUDE_EDG_TEST_ATTRIBUTES=1` and are skipped
     otherwise; several IL tests need `--no_il_lowering`.
   - This session ran the suites under amd64 emulation: per commit,
     `changes,regressions`, the C tests of `imported`, and fail-before /
     pass-after.
   - A full `imported,edg,cwg,stdlib` run at J, in container
     `tender_maxwell`, found a front-end crash in the series (§3, K). K fixes
     it. That run is still going, and its results are for J. The full suite
     at K has **not** been run: run it before upstreaming.
5. **Docker containers: done.** On 2026-10-15 the owner had the session kill
   `charming_shannon` (stale) and `tender_maxwell` (the full suite at J),
   with the host scripts driving them (`.tmp/agent-m2/evidence_run10.sh`,
   `edg-docker-test`). Standing rule from then on: a session kills any
   container running a suite that is behind the current fork tip, without
   asking. The fork's `.tmp/agent-m2/` remains; remove it when convenient.

## 3. The EDG fork series (pinned at K)

Base: upstream `034385ba0a`. All commits are on branch `cerastes` of
`~/Documents/compiler`, tagged `[GH #?]`, each with a `src/Changes` entry and
tests in `tests/tests/changes/GH-*`.

| Commit | SHA | What |
|---|---|---|
| A | 822c8f35f8 | attribute namespaces recognized only on an exact name match (prefix bug) |
| FILT | 86b5465978 | test filter normalizes `**possible non-file-scope ptr** (0x…)` in il_display output |
| B | ae98d79a22 | `register_attribute_namespace`, `ak_registered`, closed namespaces, misplacement always an error, pack rejections, parameters after a pack, the attribute account (no silent drops) |
| C | 0dfcd606d8 | statement properties in target strings: `s:+n`, `s:+i` |
| D0 | 5acfa7f720 | attributes after an abbreviated template's parameter list |
| D | 5d53cc9a91 | `RAF_ENTITY_ATTACHED` and `RAF_MUST_MATCH`: post-parameter-list attributes go on the routine; contracts must match across declarations |
| F | 8b4d48ff56 | class-template members: attribute arguments substituted with the class's template arguments |
| E | 0687d72bb2 | signature code `R`: optional result name, binder scope always pushed (params `{k,1}`, result `{1,0}`, `this` `{0,0}`) |
| G | f1d201885f | explicit specializations match the template's must-match attributes |
| H | fa150e01c7 | result-name scope for members defined outside their class (crash fix); `[this]` captures rejected |
| I | 207da5dffd | explicit specializations of member templates with a contract rejected; pack and duplicate-error fixes |
| J | 88916714e6 | substitution of registered attribute arguments fails closed (error nodes) |
| K | 23f55e3628 | an explicit specialization declared in its class template no longer reads an uninitialized template symbol (crash fix for G/I) |

What happened along the way:
- **History.** It was rewritten (autosquash) until the cerastes pin was
  first committed (`57e6c73`, G). H, I and J were added on top. The first
  version of J, `97edf599cd`, changed overload resolution for ordinary C++
  and was amended before it was ever pinned. From now on, never rewrite
  history (0009 item 5).
- **Evidence** is in this session's scratchpad (it does not survive the
  session):
  - fail-before against the parent commit for every new test, saved per
    commit;
  - `changes,regressions` matching the expectations after every commit;
  - the `imported` C loop tests;
  - mutants for B, C, D, F, G, H/I and J.

  The `upstream/attribute-registration.md` draft (§10) summarizes it.
- **K (2026-10-15)** came from the full-suite run at J.
  - **The bug.** `prototype_routine_of_template_instance` set its out
    parameters only on success. G stored the uninitialized
    `template_sym` for explicit specializations whose template is not
    known yet, and I dereferenced it.
  - **Where it crashed.** The x86-64 ASan build crashed on
    `edg/cmeerw/gnats-23971*`, `edg/daveed/d00244a`, `edg/rma/ms_cpgen01`
    and `EDGcpfe9529`. Native arm64 (stock `cpfe` and `cerastes-fe`)
    happened not to crash.
  - **Evidence** (in the fork's `.tmp/agent-k/`, the README's agent temp
    convention):
    - fail-before: J's binary SEGVs on the new
      `changes/GH-spec-in-class` in all four modes;
    - pass-after in both configurations, with its `.rto` files recorded;
    - all eight crashing `edg` tests pass;
    - `changes,regressions`: the default configuration matches the
      expectations. `edg_x86_64_cp` matches J's own run line for line,
      except that `EDGcpfe9529` (an abort at J) passes.
  - **Build directories.** The fixed x86 build was `build/gcc-k`, configured
    with `linux-gcc-debug` from the worktree `build/wt-k`. Both were
    removed after K was committed.
  - **`.tmp/agent-k/` is kept on purpose**, although the fork's
    `.tmp/README.md` asks agents to remove their temp directories, because
    it is K's only evidence. It is trimmed to 360 KB: `failbefore-J.txt`,
    the run scripts, and `runs2/` with the suite logs, the diffs and the
    archives of the tests involved. Remove it after reviewing K.
- **`edg_x86_64_cp` differences.** Besides load-dependent lines, about 85
  `EDGcpfe*` `FAIL` lines of `changes` and two lines of `regressions`
  differ from the expectations in this container, at J and at K alike.
- **`edg_x86_64_cp` flakiness.** Every full run of that configuration
  showed 70–110 load-dependent `ABORT`/`OUTPUT MISMATCH` lines (H 103, I 112,
  J 86) that pass when re-run individually. The default configuration
  matched the expectations every time.
- **Upstream material for the owner:** `internal/upstream/attribute-registration.md`
  (rewritten as "a working implementation, proposed for upstream"), with
  six adjacent-issue drafts. The owner files everything; sessions never
  open upstream issues, PRs or Discussions.

## 4. Things to look at again (M2)

1. **The audits found real bugs at every round.** Silent drops, wrong
   attachment, wrong parameter numbering after packs, crashes, and an EDG
   change that altered ordinary overload resolution. Keep this order:
   implement, have the auditor run probes against the binaries, fix.
   - Fork changes must run the `imported` and `edg` suites, not only
     `changes,regressions`, before the pin moves (CP3 part 8).
2. **Known EDG limitations** (each documented in `ext_intf.rst`, 0006, 0007
   and the user docs):
   - **Member-template prototype of a class instance.** It has no copy of a
     contract that is written only on the member template's out-of-class
     definition. cerastes reads contracts from the root template (walker:
     `contract from root template`).
   - **Explicit specializations of member templates** of class templates are
     rejected when the primary has a contract.
   - **Lambdas in function-template contracts** cannot name a template
     parameter: `forall` over `T` arrays fails at instantiation with
     `undefined_identifier`. Class-template members are unaffected.
   - **Must-match compares with EDG expression equivalence.** `this->a`
     against `a`, and any lambda (every `forall`), compare unequal, so such
     contracts are written once.
   - **GNU emulation:** `extern "C"` redeclarations in two namespaces escape
     must-match. The driver refuses GNU emulation and C before C23.
3. **What the cerastes checks cannot see** (0006 item 10 "Known gap"):
   - dependent constructs in never-instantiated templates (a `fold`/`unfold`
     callee, a dependent predicate return type, dependent calls of
     predicates/pure functions from code);
   - calls inside annotations (M3);
   - `old` in `pure`/`predicate` bodies (M3/M6);
   - tampering with `<cerastes.h>`'s macros (redefining `CERASTES_CHECK`,
     pre-defining `CERASTES_H`): unsupported, not detected.
4. **The walker listing is the IL evidence for annotations.**
   - The goldens were verified per routine and per statement against
     `--il_display`, by the auditor's oracle
     (`scratchpad/auditor/cp4b/tools/au_oracle.py`, adopted by il-analyst)
     and by differential mutation for statement attributes (il_display does
     not print statement attributes).
   - The oracle lives in the scratchpad. A future printer change needs it
     rebuilt (it is about 300 lines of Python: per-routine multisets with
     names and full positions).
5. **Name-based identification of spec functions** in never-instantiated
   templates (`tpck_unknown_function`/`tpck_template_ref` callees).
   - The CP6 version looked for a using-declaration or using-directive in
     effect. CP7 found it missed directives at block scope, at namespace
     scope and inside a qualifier namespace (`X::old`).
   - It now fails closed (`dependent_spec_callee` in
     `src/backend/annotation_checks.cpp`). An unqualified `old`, `owns`,
     `reads`, `forall` or `owns_range` counts as a spec call whenever
     namespace `::cerastes` exists in the unit. A qualified one counts
     unless its qualifier is a class, or a namespace that cannot reach
     `::cerastes` through using-directives and using-declarations.
   - Cost: a user function named `old` called unqualified from a template
     in a unit that includes `<cerastes.h>` is rejected; qualify it
     (`mine::old`). The Annotations page says so.
6. **The `__CERASTES__` guard** (CP6 item 1). `<cerastes.h>` keys on
   `__has_cpp_attribute(cerastes::requires)` (C23: `__has_c_attribute`), so
   `#undef __CERASTES__` no longer drops contracts. The source text scan
   still reports any change of the macro (`cerastes_macro_changed`) and fails
   closed when it cannot read a file (`source_scan_incomplete`).
7. **Self-test time** is about 80 s (673 tests); `check.sh` takes about
   1 min 45 s.
8. **Open audit items (CP7, CP8), deferred to M3.** None lets an annotation
   or a spec call through silently in the cases the tests pin. Each is
   either a usability gap or a tampering path that is unsupported.
   - **Header tampering not detected:** `#undef __has_cpp_attribute`, and
     `#define requires`/`#define cerastes` before `<cerastes.h>`. The text
     scan looks only for changes of `__CERASTES__`. It ignores string
     literals, and `--trigraphs` is not refused.
   - **spec_call_in_code escapes:**
     - static locals' initializers;
     - pointers to member functions (`&S::ok` of a predicate member);
     - `constexpr` predicates evaluated in constant contexts
       (`int arr[q(3)]` is folded before the back end sees it).

     Consider rejecting `constexpr`/`consteval` on `predicate`/`pure`.
   - **The shadow checks.** What they miss is listed in 0006 item 10
     (`this` captured by a lambda; for `contract_name_hides_member`,
     enclosing-class members, base members hidden later, local classes,
     in-class friends, later enumerators and later anonymous members).
     `invariant_shadowed_by_loop_variable` fails closed: `Cfg::i` and
     `Mode::i` are rejected, and the user renames the loop variable.
   - **`<cerastes.h>` included inside a namespace**
     (`namespace foo { #include <cerastes.h> }`) is not diagnosed, and
     `foo::cerastes::owns` is not treated as a spec function.
   - **`analysis_limit_reached`** is reachable only through the self-test
     hook; no source reaches the budget.

## 5. Carried over from M1 (still open)

Unchanged from the M1 handoff (`git show c0b7ef2:internal/handoff.md`, §3):
1. Generated EDG headers come from the stock build (now the fork's).
2. Shared-expression budget instead of a visited set. The walker's M1 body
   walk still caps at 1,000,000 and warns. The M2 tree printer uses a real
   visited set, and the checks fail closed on exhaustion (CP6 item 10).
3. Lambda names come from the `output_name` hooks. M2 names closures by
   their own original position.
4. Primary-file filter. M2 checks every annotated routine in every file and
   warns about cerastes attributes outside the primary file. M3 still needs
   a policy for user headers in verification.
5. Goldens are tied to the toolchain (libc++ inline namespace). There are no
   #2803 warnings any more.
6. Facts il_display cannot show (statement attributes, some constants): use
   the walker and the differential method.
7. Default arguments not evaluated; folded constants. M3's integer-semantics
   record must say how folded constants are checked.
8. No helper headers next to tests (`tests/` source-like files are errors).
   M2 used linemarkers instead.
9. `<tptk_unknown>` rendering of dependent types.

## 6. For M3

- **Verify mode:**
  - unrecognized attribute namespaces and names (`[[ceraste::…]]`) become
    errors (today only EDG's #2803 warning);
  - calls in annotations to anything that is not `pure`, `predicate` or a
    spec function are rejected, including implicit ones (overloaded
    operators, conversions, constructors);
  - a call to a function with no visible contract and no body in the unit
    is rejected.
- **Contracts across translation units:** a contract manifest per unit; a
  multi-unit verification fails when two units disagree (0006 item 6). Until
  then, a single-unit result states that assumption.
- **Parameters** in `ensures` denote entry values. Reference parameters read
  the referent on exit, and `old(x)` reads it on entry (0006 item 5).
- **Function templates called only inside contracts** are not instantiated
  (unevaluated operands). `pure`/`predicate` templates may lack instance
  bodies.
- **Explicit specializations inherit accumulating attributes** (`pure`)
  from the template. Check the purity of every body that is treated as
  pure.
- **Vector types reach the IL** since EDG bug 0007's fix is pinned
  (2026-10-15; `GNU_VECTOR_TYPES_ALLOWED`). The walker lists them through
  its generic path, and contracts with vector builtins already fail closed
  (`unsupported_annotation_expression`). The encoder must accept only an
  allow-list of type kinds and operator kinds and fail closed on everything
  else (invariant 3):
  - the new type kinds are `tk_vector`, `tk_scalable_vector`,
    `tk_scalable_vector_count`, `tk_riscv_vector`, `tk_mfp8`,
    `tk_float8e4m3` and `tk_float8e5m2`;
  - vector arithmetic uses the ordinary operator kinds (`eok_add`,
    `eok_multiply`, … with `type_kind=tk_vector`, `eok_vector_fill` inside),
    besides the `eok_vector_*` kinds, so checking operator kinds alone is
    not enough.
- **CI session before M3** (owner accepted M1.5 question 7): a decision
  record; a macOS runner with a pinned Xcode first, x64 Docker if the
  goldens drift; then `check.sh` in CI. The CI needs the fork pushed.
- **Session start** (0009): bring the fork's `cerastes` branch up to date
  with upstream `main` by merge (never rebase), move the pin, run
  `check.sh`, and review golden diffs.

## 7. For M4 (unchanged from M1, plus M2)

- **Eligibility must key on a dependent result type** (`tptk_unknown`), not
  on `type_kind` alone: in a prototype, `v[0]` on a dependent `vec<T, 4>` (or
  on any dependent `V v`) is `eok_subscript` with `type_kind=tk_pointer` and
  type `<tptk_unknown>`, which plan.md's "tk_unknown operations" would not
  catch (bug 0007 review, 2026-10-15).

The M1 notes (`git show c0b7ef2:internal/handoff.md`, §6) all still hold:
- `(T)<literal>` is a `tpck_expression` constant;
- `type_kind` is not a dependence test;
- telling prototypes from instances;
- constraints;
- VeriFast parity.

New from M2:
- generic proofs read contracts from the root template, never from an
  instance's attribute list;
- members of explicit or partial class specializations carry their own
  contracts: fail closed;
- explicit specializations follow must-match with the primary;
- fail closed on dependent constructs in never-instantiated templates (§4
  item 3);
- `forall` over `T` in function-template contracts is not available (§4
  item 2).

## 8. Friction (process and tooling)

- **The team.** edg-backend (EDG series, then L2/L3), il-analyst (probes,
  L1, positives and goldens, docs, upstream draft), test-mutator (harness,
  negatives) and the Opus auditor (CP1 to CP6, eight passes on the EDG
  series).
  - File ownership was disjoint.
  - `SendMessage` kept each agent's context across many rounds.
  - Briefs in the scratchpad (`m2-*-design.md` plus appended ADDITIONS
    sections) survived compaction.
- **Mid-session owner decisions** (the fork pin, must-match, `forall`) were
  absorbed by appending "OVERRIDE" sections to the briefs and updating the
  records at once.
- **Mistakes caught:**
  - an orchestrator commit swept a staged submodule pointer into a records
    commit (fixed by splitting a local commit); always commit with explicit
    paths;
  - agents ran `git checkout --detach` in the fork while docker evidence ran
    there (same commit, harmless);
  - an orphaned evidence script switched the fork checkout for two hours
    once;
  - `sed -i` was used twice on scratch/EDG files.
- **Permission laundering refused.** An agent denied `docker kill` asked the
  orchestrator to run it; that was refused and surfaced to the owner.
- **EDG test infrastructure:**
  - `fn` tests pass on any non-zero exit, so every case needs its own
    anchored `//match_regex`;
  - `//require:` can turn a fail-before into a SKIP;
  - clang mode in the docker test build skips uninstantiated template
    bodies, unlike the cerastes configuration.

## 9. M2 acceptance (commands and results)

| # | Command | Result |
|---|---|---|
| 1 | `scripts/check.sh` on `main` (pin K) | green (835 passed, 1 skipped; 675 self-tests; docs 0 problems) |
| 2 | `python3 scripts/run_tests.py tests/annotations doc/modules/ROOT/examples` | 791 passed; native-ok runs: Apple clang 247, GCC 247 |
| 3 | `CERASTES_EDG_SRC=~/Documents/compiler scripts/check.sh` on `main` | fails at "edg tree" (override refused on `main`) |
| 4 | fork per-commit fail-before/pass-after; suites | §3: every commit; full suite at J found the crash fixed by K; full suite at K is the owner's step (§2) |
| 5 | `scripts/build_docs.sh && python3 scripts/check_doc_examples.py --html build/doc` | 0 warnings; 6 HTML files, 60 code blocks, 0 problems |
| 6 | `git status --porcelain --untracked-files=all`; `git -C third_party/edg status --porcelain` | clean; clean |
| 7 | audits | CP1, CP1b, CP2a, CP3 parts 1–8, CP4, CP5, CP6, CP7, CP8, CP8b: every blocking finding fixed; open should-fix items in §4 item 8 |
| 8 | website link | pending the owner's push (`d313c84`) |

## 10. Questions for the owner

Each has a recommended answer. Answer inline, as before.

1. **Push order** (§2): fork `cerastes`, then cerastes `main`, then the
   website. *Recommended: as listed.*
2. **Explicit specializations** must match the primary template's contract
   (or carry none); members of class specializations carry their own. This
   was decided by the orchestrator, extending your must-match rule.
   *Recommended: keep it. Independent specialization contracts would need an
   M5-style subtyping obligation.*
3. **Explicit specializations of member templates** with a contract on the
   primary are rejected (invariant 3, after three EDG rounds did not get
   them right). *Recommended: keep it for v0; revisit in M4.*
4. **Language modes:** cerastes refuses GNU emulation and C before C23.
   *Recommended: keep it.*
5. **Lambdas in function-template contracts** cannot name `T` (an EDG
   limitation). *Recommended: accept for v0; an EDG fix (lambda rescan with
   declaration info, without crashes) is a later fork commit.*
6. **Fork commits carry `Co-Authored-By: Claude Sonnet 5.5`.** EDG's
   `CONTRIBUTING.md` asks contributors to understand and defend AI-assisted
   work. *Recommended: keep the trailer, and review the series before
   upstreaming.*
7. **Brand kit:** the loose `cerastes-lockup-on-light.svg` and
   `cerastes-avatar-500.png` also differ from the kit's copies (you asked
   only about on-dark). *Recommended: replace them with the kit's.*
8. **Dependabot** covers GitHub Actions *and* npm in `/doc`, following your
   int128 template; the earlier recommendation was Actions only.
   *Recommended: keep both.*
9. **Upstream:** when to open the Discussion, and which EDG bugs to file.
   Since 2026-10-15 the bugs are in `internal/edg_bugs/` (0001–0007), in the
   format of edgcpp/compiler#252, each reproduced on upstream `main`. 0001
   and 0002 have fix branches `0001` and `0002` in the fork clone (not
   pushed). The K&R internal error and out-of-class member substitution did
   not reproduce on upstream and are not filed. *Recommended: 0005–0007
   first (they block every libc++ header on macOS), then 0003/0004, then
   0001/0002 with their branches.*
10. **Docker containers:** answered (killed, and a standing rule; §2 step 5).
11. **Fork commit K** (`23f55e3628`) fixes a crash that G and I introduced
    for explicit specializations declared inside a class template (§3). It
    was added on top: the history is not rewritten (0009). *Recommended:
    review it with G and I, and run the full `all` suite at K on the
    x86-64 box before upstreaming the series.*
