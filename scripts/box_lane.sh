#!/usr/bin/env bash
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# The x86-64 box's lanes (internal/handoff.md section 6, "The box from M4.5e1d on"): a lane is a copy of
# a cerastes checkout under ~/<CERASTES_BOX_DIR>/<lane> on the box, with a container of its own whose
# /work/cerastes is that directory.  This script is the commands M4.5e1d typed by hand.
#
# Usage: scripts/box_lane.sh [--dry-run] <subcommand> ...
#   snap <name> [<base>] [-- <path>...]
#                          commit the working tree as a commit object (a temporary index, no branch moves,
#                          the working tree and the real index untouched), parent <base> (default HEAD),
#                          into refs/tmp/<name>; prints the ref.  With paths: only those paths differ from
#                          <base> (use it when other agents have uncommitted work in the same worktree)
#   setup <lane>           make the lane (a cp -a of the seed lane, CERASTES_BOX_SEED) if it is missing,
#                          and a container cerastes-e1e1-<lane> from the image (docker run --init)
#   ship <lane> <ref>      bundle <ref> (a branch or a ref such as refs/tmp/<name>), copy the bundle to the
#                          box, fetch it into the lane and check it out DETACHED with -f: the lane is a
#                          mirror, its uncommitted work is discarded
#   build <lane> [--fast]  scripts/bootstrap.sh --build-only in the lane's container (--fast: only ninja
#                          -C build/cmake, for an edit under src/ that changes no pin or option)
#   run <lane> <command>   <command> in the lane's container, in /work/cerastes, after . build/env.sh
#   --self-test            exercise every subcommand with --dry-run against the expected command lines and
#                          check them; needs no box and no network
#
# Each leg prints its wall time on stderr (box_lane: <leg> <n>s).  --dry-run prints the commands, one per
# line after "+ ", and runs nothing.
#
# Environment (defaults): CERASTES_BOX (matt@clang4.cpp.al), CERASTES_BOX_KEY (~/.ssh/id_rsa),
# CERASTES_BOX_DIR (cerastes-e1e1, under the box's home), CERASTES_BOX_PREFIX (cerastes-e1e1-, the
# container name prefix), CERASTES_BOX_IMAGE (cerastes-e1d:26.04), CERASTES_BOX_SEED (the box path that
# `setup` copies; default ~/<CERASTES_BOX_DIR>/work), CERASTES_BOX_JOBS (32).

set -euo pipefail

box="${CERASTES_BOX:-matt@clang4.cpp.al}"
key="${CERASTES_BOX_KEY:-$HOME/.ssh/id_rsa}"
dir="${CERASTES_BOX_DIR:-cerastes-e1e1}"
prefix="${CERASTES_BOX_PREFIX:-cerastes-e1e1-}"
image="${CERASTES_BOX_IMAGE:-cerastes-e1d:26.04}"
seed="${CERASTES_BOX_SEED:-\$HOME/$dir/work}"
jobs="${CERASTES_BOX_JOBS:-32}"
dry=0

die() {
  echo "box_lane.sh: $*" >&2
  exit 2
}

usage() {
  sed -n '5,/^[^#]/p' "$0" | sed '$d' | sed 's/^# \{0,1\}//'
}

# Run (or, with --dry-run, print) a local command.
local_cmd() {
  if [[ "$dry" == 1 ]]; then
    printf '+'
    printf ' %q' "$@"
    printf '\n'
  else
    "$@"
  fi
}

# Run a command line on the box: one string the box's shell parses.
box_cmd() {
  local line="$1"
  if [[ "$dry" == 1 ]]; then
    printf '+ ssh %s: %s\n' "$box" "$line"
  else
    ssh -i "$key" -o BatchMode=yes "$box" "$line"
  fi
}

# Run a command line in the lane's container: docker exec on the box.
lane_cmd() {
  local lane="$1" line="$2"
  box_cmd "docker exec ${prefix}${lane} bash -c $(printf '%q' "cd /work/cerastes && $line")"
}

check_lane() {
  [[ "$1" =~ ^[a-z0-9]+$ ]] || die "lane name must be lower-case letters and digits: '$1'"
}

# Print a leg's wall time on stderr.
timed() {
  local leg="$1" start
  shift
  start="$(date +%s)"
  "$@"
  echo "box_lane: $leg $(($(date +%s) - start))s" >&2
}

cmd_snap() {
  local name="$1" base=HEAD idx tree commit
  local -a paths=()
  shift
  if [[ $# -gt 0 && "$1" != -- ]]; then
    base="$1"
    shift
  fi
  if [[ $# -gt 0 ]]; then
    [[ "$1" == -- ]] || die "snap <name> [<base>] [-- <path>...]"
    shift
    paths=("$@")
    [[ ${#paths[@]} -gt 0 ]] || die "snap: -- needs at least one path"
  fi
  [[ "$name" =~ ^[A-Za-z0-9._-]+$ ]] || die "snap name: letters, digits, . _ - only: '$name'"
  if [[ "$dry" == 1 ]]; then
    printf '+ GIT_INDEX_FILE=<tmp> git read-tree %s; git add -A%s; git write-tree; git commit-tree -p %s; git update-ref refs/tmp/%s\n' \
      "$base" "${paths[*]:+ -- ${paths[*]}}" "$base" "$name"
    return
  fi
  idx="$(mktemp -u "${TMPDIR:-/tmp}/box_lane_index.XXXXXX")"
  tree="$(GIT_INDEX_FILE="$idx" bash -c 'git read-tree "$1" && shift && git add -A -- "${@:-.}" && git write-tree' _ "$base" ${paths[@]+"${paths[@]}"})"
  rm -f "$idx"
  commit="$(git commit-tree "$tree" -p "$base" -m "snapshot $name (not a session commit)")"
  git update-ref "refs/tmp/$name" "$commit"
  echo "refs/tmp/$name"
}

cmd_setup() {
  local lane="$1"
  check_lane "$lane"
  box_cmd "test -d \$HOME/$dir/$lane || cp -a $seed \$HOME/$dir/$lane"
  box_cmd "docker inspect ${prefix}${lane} >/dev/null 2>&1 || docker run -d --init --name ${prefix}${lane} -v \$HOME/$dir/$lane:/work/cerastes -e CERASTES_CI_PROFILE=ubuntu-26.04 -e CERASTES_JOBS=$jobs $image sleep infinity"
}

cmd_ship() {
  local lane="$1" ref="$2" full name bundle
  check_lane "$lane"
  if [[ "$dry" == 1 ]]; then
    full="$ref"
  else
    full="$(git rev-parse --symbolic-full-name "$ref")"
    [[ -n "$full" ]] || die "$ref is not a branch or a ref (a bundle needs a ref name; use 'snap')"
  fi
  name="${full##*/}"
  bundle="${TMPDIR:-/tmp}/box_lane_${name}.bundle"
  timed bundle local_cmd git bundle create "$bundle" "$full"
  timed copy local_cmd scp -q -i "$key" "$bundle" "$box:$dir.$name.bundle"
  timed fetch box_cmd "git -C \$HOME/$dir/$lane fetch -q \$HOME/$dir.$name.bundle $full:refs/remotes/bundle/$name && git -C \$HOME/$dir/$lane checkout -q -f --detach refs/remotes/bundle/$name && git -C \$HOME/$dir/$lane log --oneline -1"
  if [[ "$dry" != 1 ]]; then
    rm -f "$bundle"
  fi
}

cmd_build() {
  local lane="$1" fast="${2:-}"
  check_lane "$lane"
  if [[ "$fast" == --fast ]]; then
    timed build lane_cmd "$lane" ". build/env.sh && ninja -C build/cmake"
  elif [[ -z "$fast" ]]; then
    timed build lane_cmd "$lane" "scripts/bootstrap.sh --build-only"
  else
    die "build: unknown option $fast"
  fi
}

cmd_run() {
  local lane="$1" line
  shift
  check_lane "$lane"
  [[ $# -gt 0 ]] || die "run: a command is required"
  line=". build/env.sh && $(printf '%q ' "$@")"
  timed run lane_cmd "$lane" "$line"
}

# The expected command lines, checked without a box: each subcommand under --dry-run, its output
# against the text written out here.
# shellcheck disable=SC2016
# (the expected lines hold a literal $HOME, which the box expands)
self_test() {
  local self out want fail=0
  self="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/$(basename "${BASH_SOURCE[0]}")"
  check() {
    local label="$1" want_text="$2" got
    shift 2
    got="$(CERASTES_BOX=b@h CERASTES_BOX_KEY=/k CERASTES_BOX_DIR=d CERASTES_BOX_PREFIX=p- CERASTES_BOX_IMAGE=img \
      CERASTES_BOX_JOBS=4 bash "$self" --dry-run "$@" 2>/dev/null)"
    if [[ "$got" != "$want_text" ]]; then
      printf 'box_lane.sh --self-test: %s differs\n--- want\n%s\n--- got\n%s\n' "$label" "$want_text" "$got" >&2
      fail=1
    fi
  }
  want='+ ssh b@h: test -d $HOME/d/il || cp -a $HOME/d/work $HOME/d/il
+ ssh b@h: docker inspect p-il >/dev/null 2>&1 || docker run -d --init --name p-il -v $HOME/d/il:/work/cerastes -e CERASTES_CI_PROFILE=ubuntu-26.04 -e CERASTES_JOBS=4 img sleep infinity'
  check setup "$want" setup il
  want='+ git bundle create /tmp/box_lane_x.bundle refs/tmp/x
+ scp -q -i /k /tmp/box_lane_x.bundle b@h:d.x.bundle
+ ssh b@h: git -C $HOME/d/edg fetch -q $HOME/d.x.bundle refs/tmp/x:refs/remotes/bundle/x && git -C $HOME/d/edg checkout -q -f --detach refs/remotes/bundle/x && git -C $HOME/d/edg log --oneline -1'
  TMPDIR=/tmp check ship "$want" ship edg refs/tmp/x
  want='+ ssh b@h: docker exec p-edg bash -c cd\ /work/cerastes\ \&\&\ scripts/bootstrap.sh\ --build-only'
  check build "$want" build edg
  want='+ ssh b@h: docker exec p-edg bash -c cd\ /work/cerastes\ \&\&\ .\ build/env.sh\ \&\&\ ninja\ -C\ build/cmake'
  check build-fast "$want" build edg --fast
  want='+ ssh b@h: docker exec p-work bash -c cd\ /work/cerastes\ \&\&\ .\ build/env.sh\ \&\&\ build/cerastes\ verify\ t.c\ --std=c23\ '
  check run "$want" run work build/cerastes verify t.c '--std=c23'
  want='+ GIT_INDEX_FILE=<tmp> git read-tree HEAD; git add -A; git write-tree; git commit-tree -p HEAD; git update-ref refs/tmp/s1'
  check snap "$want" snap s1
  want='+ GIT_INDEX_FILE=<tmp> git read-tree abc; git add -A -- a b; git write-tree; git commit-tree -p abc; git update-ref refs/tmp/s2'
  check snap-paths "$want" snap s2 abc -- a b
  # the real snap, with and without paths, in a scratch repository (an empty paths array must not trip `set -u`)
  local scratch
  scratch="$(mktemp -d "${TMPDIR:-/tmp}/box_lane_selftest.XXXXXX")"
  (
    cd "$scratch"
    git init -q .
    echo x > f
    git add f
    git -c user.name=t -c user.email=t@t commit -q -m init
    echo y > g
    [[ "$(bash "$self" snap t1)" == "refs/tmp/t1" ]] && [[ "$(bash "$self" snap t2 HEAD -- g)" == "refs/tmp/t2" ]] &&
      [[ "$(git ls-tree -r --name-only refs/tmp/t1 | tr '\n' ' ')" == "f g " ]] &&
      [[ "$(git ls-tree -r --name-only refs/tmp/t2 | tr '\n' ' ')" == "f g " ]]
  ) || {
    printf 'box_lane.sh --self-test: a real snap (no paths / paths) failed\n' >&2
    fail=1
  }
  rm -rf "$scratch"
  out="$(CERASTES_BOX=b@h bash "$self" --dry-run build 'Bad Lane' 2>&1 || true)"
  want="box_lane.sh: lane name must be lower-case letters and digits: 'Bad Lane'"
  if [[ "$out" != "$want" ]]; then
    printf 'box_lane.sh --self-test: bad lane differs\n--- want\n%s\n--- got\n%s\n' "$want" "$out" >&2
    fail=1
  fi
  if [[ "$fail" == 0 ]]; then
    echo "box_lane.sh --self-test: ok"
  fi
  return "$fail"
}

args=()
for a in "$@"; do
  case "$a" in
    --dry-run) dry=1 ;;
    *) args+=("$a") ;;
  esac
done
set -- "${args[@]+"${args[@]}"}"
[[ $# -gt 0 ]] || {
  usage
  exit 2
}
sub="$1"
shift
case "$sub" in
  -h | --help) usage ;;
  --self-test) self_test ;;
  snap) [[ $# -ge 1 ]] || die "snap <name> [<base>] [-- <path>...]"; cmd_snap "$@" ;;
  setup) [[ $# -eq 1 ]] || die "setup <lane>"; cmd_setup "$@" ;;
  ship) [[ $# -eq 2 ]] || die "ship <lane> <ref>"; cmd_ship "$@" ;;
  build) [[ $# -ge 1 && $# -le 2 ]] || die "build <lane> [--fast]"; cmd_build "$@" ;;
  run) [[ $# -ge 2 ]] || die "run <lane> <command...>"; cmd_run "$@" ;;
  *) die "unknown subcommand '$sub' (see --help)" ;;
esac
