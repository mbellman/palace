#pragma once

#include <string>

namespace Gamma {
  std::string Gm_LoadFileContents(const char* path);
  void Gm_WriteFileContents(const char* path, const std::string& contents);
}