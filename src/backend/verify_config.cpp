// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "verify_config.h"

#include <climits>
#include <cstdlib>

namespace cerastes {

VerifyConfig& verify_config() {
  static VerifyConfig c;
  return c;
}

int front_end_exit_status(const VerifyOutput& out, int edg_status) {
  if (out.internal_inconsistency) return 3;
  return edg_status != 0 ? 2 : 0;
}

std::string canonical_path(const std::string& path) {
  if (path.empty()) return "";
  char buf[PATH_MAX];
  if (::realpath(path.c_str(), buf) == nullptr) return "";
  return buf;
}

bool path_is_under(const std::string& path, const std::string& dir) {
  if (dir.empty() || path.size() <= dir.size()) return false;
  if (path.compare(0, dir.size(), dir) != 0) return false;
  // Whole components: the next character must start a new one, unless the directory is "/" itself.
  return dir.back() == '/' || path[dir.size()] == '/';
}

std::vector<std::string> system_directories(const std::vector<std::string>& dirs,
                                            const std::vector<std::string>& user_dirs) {
  std::vector<std::string> users;
  users.push_back(canonical_path("/usr/local/include"));
  for (const std::string& u : user_dirs) users.push_back(canonical_path(u));
  std::vector<std::string> out;
  for (const std::string& d : dirs) {
    std::string c = canonical_path(d);
    if (c.empty()) continue;
    bool user = false;
    for (const std::string& u : users)
      if (!u.empty() && u == c) user = true;
    if (user) continue;
    bool seen = false;
    for (const std::string& o : out)
      if (o == c) seen = true;
    if (!seen) out.push_back(c);
  }
  return out;
}

bool is_system_file(const std::vector<std::string>& system_dirs, const std::string& cerastes_header,
                    const std::string& path) {
  std::string c = canonical_path(path);
  if (c.empty()) return false;  // cannot be resolved: not a system file (fail closed)
  if (!cerastes_header.empty() && c == cerastes_header) return true;
  for (const std::string& d : system_dirs)
    if (path_is_under(c, d)) return true;
  return false;
}

bool is_system_file(const std::string& path) {
  const VerifyConfig& c = verify_config();
  return is_system_file(c.system_dirs, c.cerastes_header, path);
}

}  // namespace cerastes
