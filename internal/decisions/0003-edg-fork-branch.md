# 0003 — EDG fork branch for cerastes changes

- **Status:** accepted
- **Milestone:** after M1 (2026-10-13), ahead of M2

## Context

Invariant 5 (CLAUDE.md, plan.md §1) says `third_party/edg` only ever pins an
upstream `edgcpp/compiler` commit. EDG changes that cerastes needs are
developed as small commits, each with a test, on a `cerastes` branch of a
fork, and land upstream before the pin moves. Decision 0001 (departure 1)
records why the submodule points at upstream rather than at a fork: anyone
must be able to build cerastes.

M1 needed no EDG change (decision 0002). M2 does. Registering the
`[[cerastes::…]]` attributes, so that EDG parses and type-checks their
arguments as C++ expressions, means changing `src/attribute.c`. The
attribute descriptor table and the hard-coded list of known attribute
namespaces are both there (`attribute.c:970-980`). The M1 inventory
(`internal/il-inventory.md`, check 5c) showed why the unrecognized-attribute
path cannot carry contracts written after the parameter list: they land on
the function type and are lost on redeclaration. So M2 cannot avoid the
change, and the upstream cycle is M2's critical path (`internal/handoff.md` §5).

On 2026-10-13 the owner created the branch.

## Decision

1. **Where EDG changes are developed.** Every EDG source change cerastes
   needs is made on branch **`cerastes`** of the owner's fork,
   **https://github.com/mborland/compiler**. It is never made in
   `third_party/edg`, which stays a clean checkout of upstream
   (`git -C third_party/edg status --porcelain` is empty; `check.sh` relies
   on it).
2. **What the submodule may point at.** `third_party/edg` keeps its URL
   `https://github.com/edgcpp/compiler.git`. It is moved only to a commit
   that is on upstream `main`. A commit of this repository never records a
   submodule pointer to a fork commit, on any branch.
3. **Base of the branch.** The branch must start from the pinned upstream
   commit or from a later upstream `main` commit. That keeps every change
   upstreamable as a clean series and lets the pin move straight to the
   upstream merge.
   - When the branch was created, its tip was `90b66472` (2026-09-29,
     "Add normalization rules for …constant-expression-cxx2a.sft.cpp"). That
     is the fork's old `main`, 22 commits *behind* the pin `9847356d`, and an
     ancestor of it.
   - Before its first commit, fast-forward the branch to the pin
     (`git merge --ff-only 9847356d` on `cerastes`, then push). Rebase it
     onto upstream `main` whenever the pin moves.
   - The owner pushes this fast-forward (item 6). On 2026-10-14 the branch
     was still at `90b66472`, and upstream `main` had moved on to
     `4e739c28`.
4. **Kept apart from the CIR work.** The ClangIR back end lives on the fork's
   `cir` branch (`BACK_END_TYPE`, the ECIL writer). Nothing from `cir` is
   merged into `cerastes`, and vice versa. cerastes does not depend on
   `BACK_END_TYPE`. If it lands upstream, cerastes's configuration becomes
   `BACK_END_TYPE_EXTERNAL` (0002), with no fork involved.
5. **What each commit must be.** Each commit on `cerastes` is aimed at an
   upstream PR and follows EDG's `CONTRIBUTING.md` checklist:
   - an open upstream issue, and the commit tagged `[GH #N]`;
   - EDG style and comments;
   - a `src/Changes` entry;
   - EDG tests that fail before the change and pass after it, run with
     `edg-docker-test`, with recordings reviewed.

   EDG prefers the Docker workflow to the host toolchain, so the tests run
   where `edg-docker-test` works. The x86-64 box used for the CIR work is one
   such place.
6. **Who does what.**
   - Claude sessions may commit to the fork's `cerastes` branch. This is a
     standing permission from the owner (2026-10-13). It satisfies EDG's
     `AGENTS.md` rule ("never commit or push unless asked") for this branch
     only, not for `cir` or any other branch.
   - **The owner pushes** (answer to handoff question 1, 2026-10-13). A
     session commits in a local clone of the fork, outside this repository
     (for example `~/Documents/compiler`), and names the commits to push in
     `internal/handoff.md`. Sessions have no push credentials for the fork.
   - **Claude never opens issues, pull requests or Discussions against
     upstream `edgcpp/compiler`. The owner does all of that.** A session
     that has an upstreamable series ready writes the proposed issue and PR
     text, with the `CONTRIBUTING.md` checklist filled in, into
     `internal/handoff.md` for the owner to file. Until the owner has opened the
     issue, commits carry `[GH #?]` placeholders in place of the issue
     number.
7. **Prefer mechanisms upstream can accept.** EDG upstream has to want the
   change, so prefer general mechanisms to cerastes-specific code inside
   EDG. Examples:
   - a way for an external back end, or a command-line option, to declare
     extra attribute namespaces and attribute descriptors;
   - in preference to hard-coding `cerastes` into `attribute.c`.

   M2 records the exact design in its own decision record before writing the
   code (invariant 6).
8. **Until a change is upstream.**
   - To develop or test against an unmerged fork commit locally, add the
     fork as a remote inside `third_party/edg` and check the commit out
     there. `scripts/bootstrap.sh` leaves a submodule that is not at the
     pinned commit alone, with a warning. Restore the pin before running the
     end-of-session `check.sh`.
   - cerastes code that needs an unmerged EDG change is not merged into this
     repository's `main` until the pin has moved to an upstream commit that
     contains it. Such code may wait on a topic branch of this repository.
   - `check.sh` on `main` is always run against the upstream pin.

## Consequences

- **Upstream review gates M2.** M2's attribute work cannot be merged into
  cerastes `main` until upstream accepts the EDG side. Start the upstream
  issue, discussion and PR at the very beginning of M2. A WIP PR is accepted
  upstream for early design feedback.
- **Moving the pin** follows decision 0001's "Changing a pin" procedure. Each
  move also re-runs the EDG configuration check (`scripts/check_edg_config.sh`)
  and every IL golden test, so any IL change the new commit brings shows up as
  a reviewed golden diff.
- **Numbering.** plan.md had reserved 0003 for a front-end fallback record
  (`0003-frontend-fallback.md`), to be written only if Gate G1 failed. G1
  passed (`internal/il-inventory.md`), so this record takes 0003. A fallback
  record needed after a future pin move takes the next free number.

## Settled questions

- **Upstream interaction.** The owner files every upstream issue, Discussion
  and PR (item 6, 2026-10-13).
- **Pushes to the fork.** The owner pushes; sessions commit in a local clone
  of the fork (item 6; handoff question 1, answer B, 2026-10-13).
- **No fork pointer, on any branch** (item 2; handoff question 2,
  2026-10-13). The owner will re-evaluate this if upstream EDG review turns
  out to be slow.
