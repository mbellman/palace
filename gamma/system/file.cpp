#include <fstream>

#include "system/assert.h"
#include "system/console.h"
#include "system/file.h"

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
}