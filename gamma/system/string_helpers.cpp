#include "system/string_helpers.h"
#include "system/type_aliases.h"

namespace Gamma {
  /**
   * Gm_SplitString
   * --------------
   *
   * @todo description
   */
  std::vector<std::string> Gm_SplitString(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> values;
    u32 offset = 0;
    u32 found = 0;

    // Add each delimited string segment to the list
    while ((found = str.find(delimiter, offset)) != std::string::npos) {
      values.push_back(str.substr(offset, found - offset));

      offset = found + delimiter.size();
    }

    // Include the remaining string segment after the final delimiter
    values.push_back(str.substr(offset, str.size() - offset));

    return values;
  }

  /**
   * Gm_JoinString
   * -------------
   *
   * @todo description
   */  
  std::string Gm_JoinString(const std::vector<std::string>& segments, const std::string& delimiter) {
    std::string joined;

    for (u32 i = 0; i < segments.size(); i++) {
      joined += segments[i];

      if (i < segments.size() - 1) {
        joined += delimiter;
      }
    }

    return joined;
  }

  /**
   * Gm_TrimString
   * -------------
   *
   * @todo description
   */
  std::string Gm_TrimString(const std::string& str) {
    std::string trimmed;
    u32 start = 0;
    u32 end = str.size();

    for (;;) {
      if (str[start] == ' ') start++;
      else if (str[--end] != ' ')  break;
    }

    return str.substr(start, end - start + 1);
  }
}