#include <fstream>
#include <filesystem>

#include "system/assert.h"
#include "system/console.h"
#include "system/file.h"
#include "system/string_helpers.h"

namespace Gamma {
  std::string Gm_LoadFileContents(const char* path) {
    std::string source;
    std::ifstream file(path);

    assert(!file.fail(), "[Gamma] Gm_LoadFileContents failed to load file: " + std::string(path));

    std::string line;

    while (std::getline(file, line)) {
      source.append(line + "\n");
    }

    file.close();

    return source;
  }

  void Gm_WriteFileContents(const char* path, const std::string& contents) {
    // Ensure the directory exists
    auto pathSegments = Gm_SplitString(path, "/");

    pathSegments.pop_back();

    auto directories = Gm_JoinString(pathSegments, "/");

    std::filesystem::create_directories(directories);

    // Write to the file
    std::ofstream file(path);

    file << contents;
    file.flush();
  }
}