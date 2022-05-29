#pragma once

#include <string>
#include <vector>

namespace Gamma {
  std::vector<std::string> Gm_SplitString(const std::string& str, const std::string& delimiter);
  std::string Gm_TrimString(const std::string& str);
}