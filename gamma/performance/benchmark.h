#pragma once

#include <chrono>
#include <functional>

#include "system/type_aliases.h"

Gamma::uint64 Gm_GetMicroseconds();

namespace Gamma {
  void Gm_CompareBenchmarks(uint64 a, uint64 b);

  auto Gm_CreateTimer() {
    auto start = std::chrono::system_clock::now();

    return [&]() {
      auto end = std::chrono::system_clock::now();

      std::chrono::system_clock::duration duration = end - start;

      return duration;
    };
  };

  uint64 Gm_RepeatBenchmarkTest(const std::function<void()>& test, uint32 times = 1);
  uint64 Gm_RunBenchmarkTest(const std::function<void()>& test);
  void Gm_RunLoopedBenchmarkTest(const std::function<void()>& test, uint32 pause = 1000);
  void Gm_Sleep(uint32 milliseconds);
}