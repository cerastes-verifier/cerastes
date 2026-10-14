# 0009 — cerastes pins the owner's EDG fork

- **Status:** accepted (owner, 2026-10-14). Supersedes decision 0003 items 2,
  3 and 8, and the topic-branch policy of 0005 item 6. Changes invariant 5.
- **Milestone:** M2 (2026-10-14)

## Context

Until now, `third_party/edg` could only point at upstream `edgcpp/compiler`
commits. cerastes code that needed an unmerged EDG change had to wait on a
topic branch until upstream merged it (invariant 5; decisions 0003 and 0005).

M2's EDG series (decision 0007) grew past a small patch once the audits had
been through it. It now covers:
- registration;
- statement targets;
- entity attachment with must-match;
- result names;
- substitution fixes;
- rejecting parameter packs.

On 2026-10-14 the owner decided: "there are enough changes required to EDG
that we should run cerastes off my fork for the near future rather than
waiting for EDG to merge. You can keep fast forwarding the cerastes branch
or EDG against main at the beginning of sessions to keep things moving
along."

## Decision

1. **The submodule points at the fork.** `third_party/edg` uses the URL
   `https://github.com/mborland/compiler.git` and pins a commit on the fork's
   `cerastes` branch. That branch carries upstream `main` plus the cerastes
   series. It never carries the `cir` work (0003 item 4 still holds).
2. **The pin moves to fork commits** as soon as they are committed and
   reviewed, so M2's cerastes code lands on `main`.
   - The topic branch `m2-annotations` is dropped.
   - Uncommitted EDG work is still built through `CERASTES_EDG_SRC` (0005).
     `check.sh` on `main` still refuses that, so the final check always runs
     against the pin.
3. **Who pushes.** Sessions commit to the fork's `cerastes` branch in the
   local clone (`~/Documents/compiler`), and the owner pushes it (0003 item 6,
   unchanged).
   - Until the owner pushes, the pin can name a commit that exists only in
     the local clone.
   - To build it, the submodule fetches from the clone through a remote named
     `fork-local` (`git -C third_party/edg remote add fork-local
     ~/Documents/compiler`).
   - The handoff then lists the pushes in order: the fork's `cerastes` branch
     first, then cerastes `main`. Otherwise a clean clone of cerastes cannot
     fetch its pin.
4. **The guard** (`scripts/edg_tree.py check`, run by `check.sh`). It fails
   unless all of these hold:
   - `.gitmodules` and the submodule's `origin` name
     `https://github.com/mborland/compiler.git`;
   - `third_party/edg` is clean and at its gitlink;
   - the gitlink is contained in `origin/cerastes`, or in
     `fork-local/cerastes`.

   When it is contained only in `fork-local/cerastes`, the report line says
   `NOT YET PUSHED`, and the handoff must name the push.
5. **Keeping up with upstream.** At the start of a session, bring the fork's
   `cerastes` branch up to date with upstream `main`. While the branch has no
   commits of its own, that is a fast-forward. Once it has commits, it is a
   **merge** of upstream `main`, never a rebase or a force-push.
   - A rebase would rewrite fork commits that earlier cerastes commits pin.
     Clones of those cerastes commits would then break.
   - Then move the pin to the new tip and run `check.sh`. Any IL change
     upstream brings shows up as a reviewed golden diff (0001 "Changing a
     pin").
6. **Upstreaming later.** The series stays upstreamable:
   - commits follow EDG's `CONTRIBUTING.md` (0003 item 5);
   - commit messages carry `[GH #?]`;
   - `internal/upstream/` keeps the Discussion and issue drafts.

   When the owner takes a change upstream, a clean PR branch can be cut from
   upstream `main` by cherry-picking. Once upstream contains everything, the
   submodule may return to upstream.

## Consequences

- Invariant 5 (CLAUDE.md, plan.md §1) now says the pin is a commit of the
  owner's fork, `cerastes` branch.
- 0003 items 2, 3 and 8 are superseded:
  - item 2: no fork pointer;
  - item 3: the branch is based on the pin;
  - item 8: changes wait for upstream.

  Items 1 (no EDG edits in `third_party/edg`), 4 (`cir` kept apart), 5
  (commit quality), 6 (who pushes) and 7 (general mechanisms) stand.
- 0005 item 6 (topic branch) is superseded. The override itself stays, for
  uncommitted EDG work.
- The user docs for annotations are published as soon as M2 is on `main` and
  pushed. They no longer wait for upstream.
- A clean clone needs the fork, so anyone building cerastes depends on the
  owner's fork being public and pushed.
