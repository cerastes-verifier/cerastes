#!/usr/bin/env python3
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Which EDG tree is cerastes built from?  (decision 0005)

Subcommands (all print to stdout; diagnostics go to stderr):

  resolve                    print `upstream <abs path>` or `override <abs
                             realpath>`.  The override is the environment
                             variable CERASTES_EDG_SRC (unset or empty:
                             upstream, ROOT/third_party/edg).  Exit 2 with a
                             message on stderr for a bad override: a relative
                             path, missing, no src/il_def.h, not the root of a
                             git work tree, or equal to / inside
                             ROOT/third_party/edg.
  stamp TREE                 print the five-line stamp of the tree at TREE:
                               tree <abs realpath>
                               head <sha40>
                               branch <name>|(detached)
                               dirty <count> <sha256 hex>
                               config <sha256 hex>
                             The dirty hash covers `git diff --binary HEAD` and
                             every untracked, non-ignored path with its content
                             hash; <count> is the number of modified (tracked)
                             plus untracked paths.  The config hash covers the
                             bytes of cmake/EdgFrontEnd.cmake followed by those
                             of src/backend/edg_il.h (ROOT-relative, a missing
                             file counts as a marker): the cerastes
                             configuration of EDG, which a binary built under
                             an older one must not pass.
  check                      the guards of decision 0005 item 5.  Prints one
                             line `EDG tree: ...` and exits 0, or prints
                             `error: ...` lines and exits 1 (exit 2 for a
                             relative CERASTES_EDG_SRC).
  describe [--verify-build]  print the same report line (exit 2 for a bad
                             override).  With --verify-build, exit 1 unless
                             the stamp file of the active kind
                             (build/edg/edg-tree.stamp, or
                             build/override/edg/edg-tree.stamp) exists and
                             equals `stamp <active tree>` now.

Every git subprocess runs without the variables that redirect git to another
repository (GIT_DIR, GIT_WORK_TREE, GIT_INDEX_FILE, ...).

Self-tests and tools point the script at another repository with the global
option `--root DIR` (given before the subcommand; default: the repository this
script lives in), or by calling the module functions with `root=DIR`.
Python >= 3.9, standard library only.
"""

from __future__ import annotations

import argparse
import hashlib
import os
import subprocess
import sys
from typing import List, Optional, Tuple

DEFAULT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EDG_URL = "https://github.com/edgcpp/compiler.git"
SUBMODULE = "third_party/edg"
ENV_VAR = "CERASTES_EDG_SRC"


class TreeError(Exception):
    """A bad EDG tree; the message is meant for the user."""


class RelativeOverride(TreeError):
    """CERASTES_EDG_SRC is not an absolute path (exit status 2 everywhere)."""


# Variables that point git at another repository, work tree or object store;
# an inherited one would make every command below look at the wrong tree.
GIT_REDIRECTS = ("GIT_DIR", "GIT_WORK_TREE", "GIT_INDEX_FILE", "GIT_OBJECT_DIRECTORY",
                 "GIT_ALTERNATE_OBJECT_DIRECTORIES", "GIT_COMMON_DIR", "GIT_NAMESPACE",
                 "GIT_PREFIX", "GIT_INTERNAL_SUPER_PREFIX", "GIT_QUARANTINE_PATH",
                 "GIT_REPLACE_REF_BASE")


def git_environment() -> dict:
    return {k: v for k, v in os.environ.items() if k not in GIT_REDIRECTS}


def git(args: List[str], cwd: str, check: bool = True) -> "subprocess.CompletedProcess[bytes]":
    proc = subprocess.run(["git"] + args, cwd=cwd, stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE, check=False, env=git_environment())
    if check and proc.returncode != 0:
        raise TreeError("git %s failed in %s: %s"
                        % (" ".join(args), cwd, proc.stderr.decode(errors="replace").strip()))
    return proc


def git_out(args: List[str], cwd: str) -> str:
    return git(args, cwd).stdout.decode(errors="replace").strip()


def upstream_path(root: str) -> str:
    return os.path.join(os.path.abspath(root), SUBMODULE)


def is_inside(path: str, parent: str) -> bool:
    path, parent = os.path.realpath(path), os.path.realpath(parent)
    return path == parent or path.startswith(parent.rstrip(os.sep) + os.sep)


def own_toplevel(path: str) -> Optional[str]:
    """The realpath of PATH's own git work tree root, or None when PATH is not
    the root of a work tree (a subdirectory of another repository does not
    count: `git -C` would silently report that repository)."""
    proc = git(["rev-parse", "--show-toplevel"], path, check=False)
    if proc.returncode != 0:
        return None
    top = proc.stdout.decode(errors="replace").strip()
    if os.path.realpath(top) != os.path.realpath(path):
        return None
    return os.path.realpath(top)


def resolve(root: str = DEFAULT_ROOT, env: Optional[dict] = None) -> Tuple[str, str]:
    """(kind, path).  Raises TreeError for a bad override."""
    env = os.environ if env is None else env
    value = env.get(ENV_VAR, "")
    if not value:
        return "upstream", upstream_path(root)
    if not value.startswith("/"):
        raise RelativeOverride("%s=%s is a relative path; use an absolute path (decision 0005)"
                               % (ENV_VAR, value))
    if not os.path.isdir(value):
        raise TreeError("%s=%s is not a directory" % (ENV_VAR, value))
    real = os.path.realpath(value)
    if not os.path.isfile(os.path.join(real, "src", "il_def.h")):
        raise TreeError("%s=%s has no src/il_def.h; it must be the root of an EDG checkout"
                        % (ENV_VAR, value))
    if own_toplevel(real) is None:
        raise TreeError("%s=%s is not the root of a git work tree" % (ENV_VAR, value))
    if is_inside(real, upstream_path(root)):
        raise TreeError("%s=%s is, or is inside, third_party/edg; leave the variable unset "
                        "to build the pin (decision 0005)" % (ENV_VAR, value))
    return "override", real


def branch_of(tree: str) -> str:
    proc = git(["symbolic-ref", "--short", "-q", "HEAD"], tree, check=False)
    name = proc.stdout.decode(errors="replace").strip()
    return name if proc.returncode == 0 and name else "(detached)"


def untracked_paths(tree: str) -> List[str]:
    out = git(["ls-files", "--others", "--exclude-standard", "-z"], tree).stdout
    return sorted(p.decode(errors="surrogateescape") for p in out.split(b"\0") if p)


def modified_paths(tree: str) -> List[str]:
    out = git(["diff", "--name-only", "-z", "--no-ext-diff", "--no-textconv", "HEAD"], tree).stdout
    return sorted(p.decode(errors="surrogateescape") for p in out.split(b"\0") if p)


def file_digest(path: str) -> str:
    h = hashlib.sha256()
    try:
        if os.path.islink(path):
            h.update(b"symlink " + os.readlink(path).encode(errors="surrogateescape"))
        else:
            with open(path, "rb") as f:
                for chunk in iter(lambda: f.read(1 << 20), b""):
                    h.update(chunk)
    except OSError as exc:
        h.update(("unreadable %s" % exc.__class__.__name__).encode())
    return h.hexdigest()


CONFIG_FILES = ("cmake/EdgFrontEnd.cmake", "src/backend/edg_il.h")


def config_digest(root: str = DEFAULT_ROOT) -> str:
    """sha256 of the bytes of CONFIG_FILES, in order (ROOT-relative)."""
    h = hashlib.sha256()
    for name in CONFIG_FILES:
        try:
            with open(os.path.join(os.path.abspath(root), name), "rb") as f:
                h.update(f.read())
        except OSError:
            h.update(b"<missing " + name.encode() + b">")
    return h.hexdigest()


def stamp(tree: str, root: str = DEFAULT_ROOT) -> str:
    real = os.path.realpath(tree)
    head = git_out(["rev-parse", "HEAD"], real)
    diff = git(["diff", "--binary", "--no-ext-diff", "--no-textconv", "HEAD"], real).stdout
    h = hashlib.sha256()
    h.update(diff)
    untracked = untracked_paths(real)
    for rel in untracked:
        h.update(b"\0untracked " + rel.encode(errors="surrogateescape") + b" "
                 + file_digest(os.path.join(real, rel)).encode() + b"\n")
    count = len(modified_paths(real)) + len(untracked)
    return "tree %s\nhead %s\nbranch %s\ndirty %d %s\nconfig %s\n" % (
        real, head, branch_of(real), count, h.hexdigest(), config_digest(root))


def gitlink(root: str) -> str:
    return git_out(["rev-parse", ":" + SUBMODULE], os.path.abspath(root))


def short(sha: str) -> str:
    return sha[:8]


def submodule_gitmodules_url(root: str) -> Optional[str]:
    gm = os.path.join(root, ".gitmodules")
    if not os.path.isfile(gm):
        return None
    proc = git(["config", "-f", gm, "--get-regexp", r"^submodule\..*\.path$"], root, check=False)
    for line in proc.stdout.decode(errors="replace").splitlines():
        key, _, path = line.partition(" ")
        if path.strip() == SUBMODULE:
            name = key[len("submodule."):-len(".path")]
            url = git(["config", "-f", gm, "--get", "submodule.%s.url" % name], root, check=False)
            return url.stdout.decode(errors="replace").strip() or None
    return None


def guard_pin(root: str) -> List[str]:
    """Item 5, first bullet: third_party/edg is clean, at its gitlink, with the
    upstream URL.  Always checked."""
    errors: List[str] = []
    sub = upstream_path(root)
    if not os.path.isdir(sub) or own_toplevel(sub) is None:
        return ["third_party/edg is not an initialized submodule; run scripts/bootstrap.sh"]
    status = git(["status", "--porcelain", "--untracked-files=all"], sub).stdout.decode(
        errors="replace").splitlines()
    if status:
        errors.append("third_party/edg is not clean (%d path%s changed, first: %s); EDG changes "
                      "belong on the fork branch and CERASTES_EDG_SRC (decisions 0003, 0005)"
                      % (len(status), "" if len(status) == 1 else "s", status[0]))
    try:
        link = gitlink(root)
    except TreeError as exc:
        errors.append(str(exc))
        link = ""
    head = git_out(["rev-parse", "HEAD"], sub)
    if link and head != link:
        errors.append("third_party/edg is at %s but the index records %s for it"
                      % (short(head), short(link)))
    if link:
        # 0003 item 2: the pin is an upstream commit, never a fork commit.
        # origin/main is what the submodule last fetched; a stale one fails
        # too, with the way out.
        up = git(["merge-base", "--is-ancestor", link, "refs/remotes/origin/main"], sub,
                 check=False)
        if up.returncode != 0:
            errors.append("the gitlink %s of third_party/edg is not contained in its "
                          "origin/main (a fork commit, or a stale origin/main: run `git -C "
                          "third_party/edg fetch origin`); the pin is only ever an upstream "
                          "commit (decision 0003)" % short(link))
    url = submodule_gitmodules_url(root)
    if url != EDG_URL:
        errors.append(".gitmodules URL of third_party/edg is %s, expected %s"
                      % (url or "<missing>", EDG_URL))
    origin = git(["config", "--get", "remote.origin.url"], sub, check=False).stdout.decode(
        errors="replace").strip()
    if origin != EDG_URL:
        errors.append("remote.origin.url of third_party/edg is %s, expected %s"
                      % (origin or "<missing>", EDG_URL))
    return errors


def report(root: str, kind: str, tree: str) -> Tuple[str, List[str]]:
    """(report line, warnings)."""
    warnings: List[str] = []
    head = short(git_out(["rev-parse", "HEAD"], tree))
    if kind == "upstream":
        dirty = bool(git_out(["status", "--porcelain", "--untracked-files=all"], tree))
        return ("EDG tree: upstream third_party/edg @ %s (gitlink, %s)"
                % (head, "DIRTY" if dirty else "clean")), warnings
    branch = branch_of(tree)
    modified, untracked = len(modified_paths(tree)), len(untracked_paths(tree))
    try:
        link = gitlink(root)
    except TreeError:
        link = ""
    ahead = None
    if link:
        ok = git(["merge-base", "--is-ancestor", link, "HEAD"], tree, check=False)
        if ok.returncode == 0:
            ahead = git_out(["rev-list", "--count", "%s..HEAD" % link], tree)
        else:
            warnings.append("the pin %s is not an ancestor of HEAD in %s" % (short(link), tree))
    where = ("%s commits on top of pin %s" % (ahead, short(link)) if ahead is not None
             else "pin %s is not an ancestor" % (short(link) if link else "<unknown>"))
    return ("EDG tree: OVERRIDE %s @ %s (branch %s, %s, %d modified, %d untracked)"
            % (tree, head, branch, where, modified, untracked)), warnings


def check(root: str = DEFAULT_ROOT, env: Optional[dict] = None) -> Tuple[int, List[str], List[str]]:
    """(exit status, stdout lines, stderr lines)."""
    root = os.path.abspath(root)
    out: List[str] = []
    err: List[str] = []
    errors = guard_pin(root)
    kind, tree = "upstream", upstream_path(root)
    try:
        kind, tree = resolve(root, env)
    except RelativeOverride as exc:
        return 2, out, ["error: %s" % exc]
    except TreeError as exc:
        errors.append(str(exc))
        kind = "bad"
    if kind == "override":
        proc = git(["symbolic-ref", "--short", "-q", "HEAD"], root, check=False)
        branch = proc.stdout.decode(errors="replace").strip() if proc.returncode == 0 else ""
        if branch == "main":
            errors.append("CERASTES_EDG_SRC is set on branch main; check.sh on main runs against "
                          "the upstream pin (decisions 0003, 0005)")
        elif not branch:
            errors.append("CERASTES_EDG_SRC is set with a detached HEAD; check.sh needs a "
                          "named topic branch other than main (decisions 0003, 0005)")
    if errors:
        err.extend("error: " + e for e in errors)
        return 1, out, err
    line, warnings = report(root, kind, tree)
    err.extend("warning: " + w for w in warnings)
    out.append(line)
    return 0, out, err


def stamp_file(root: str, kind: str) -> str:
    base = os.path.join(os.path.abspath(root), "build")
    if kind == "override":
        base = os.path.join(base, "override")
    return os.path.join(base, "edg", "edg-tree.stamp")


def describe(root: str = DEFAULT_ROOT, verify_build: bool = False,
             env: Optional[dict] = None) -> Tuple[int, List[str], List[str]]:
    root = os.path.abspath(root)
    try:
        kind, tree = resolve(root, env)
    except TreeError as exc:
        return 2, [], ["error: %s" % exc]
    line, warnings = report(root, kind, tree)
    err = ["warning: " + w for w in warnings]
    if verify_build:
        path = stamp_file(root, kind)
        try:
            with open(path, encoding="utf-8") as f:
                recorded = f.read()
        except OSError:
            return 1, [line], err + ["error: %s is missing; run scripts/bootstrap.sh" % path]
        if recorded != stamp(tree, root):
            return 1, [line], err + [
                "error: the binary under test was built from a different EDG tree state than "
                "the active one (%s differs from the stamp of %s); run scripts/bootstrap.sh"
                % (path, tree)]
    return 0, [line], err


def emit(status: int, out: List[str], err: List[str]) -> int:
    for line in out:
        print(line)
    for line in err:
        print(line, file=sys.stderr)
    return status


def main(argv: Optional[List[str]] = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("--root", default=DEFAULT_ROOT, help="repository root (self-tests)")
    sub = parser.add_subparsers(dest="cmd", required=True)
    sub.add_parser("resolve")
    p_stamp = sub.add_parser("stamp")
    p_stamp.add_argument("tree")
    sub.add_parser("check")
    p_desc = sub.add_parser("describe")
    p_desc.add_argument("--verify-build", action="store_true")
    args = parser.parse_args(argv)
    try:
        if args.cmd == "resolve":
            kind, tree = resolve(args.root)
            print("%s %s" % (kind, tree))
            return 0
        if args.cmd == "stamp":
            sys.stdout.write(stamp(args.tree, args.root))
            return 0
        if args.cmd == "check":
            return emit(*check(args.root))
        return emit(*describe(args.root, args.verify_build))
    except TreeError as exc:
        print("error: %s" % exc, file=sys.stderr)
        return 2 if args.cmd in ("resolve", "describe") or isinstance(exc, RelativeOverride) else 1


if __name__ == "__main__":
    sys.exit(main())
