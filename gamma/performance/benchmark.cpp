#include <chrono>
#include <iostream>
#include <thread>

#include "performance/benchmark.h"

Gamma::uint64 Gm_GetMicroseconds() {
  auto now = std::chrono::system_clock::now();

  return std::chrono::time_point_cast<std::chrono::microseconds>(now).time_since_epoch().count();
}

namespace Gamma {
  void Gm_CompareBenchmarks(uint64 a, uint64 b) {
    if (a > b) {
      uint32 improvement = (uint32)(100.0f * (1.0f - (float)b / (float)a));

      std::cout << a << "ms -> " << b << "ms (-" << improvement << "%)\n";
    } else {
      uint32 slowdown = (uint32)(100.0f * (1.0f - (float)a / (float)b));

      std::cout << a << "ms -> " << b << "ms (+" << slowdown << "%)\n";
    }
  }

  uint64 Gm_RepeatBenchmarkTest(const std::function<void()>& test, uint32 times) {
    // Warmup run - without this, the first timed test
    // invocation may take longer than usual. (Might be
    // instruction cache-related)
    test();

    auto getTime = Gm_CreateTimer();

    for (uint32 i = 0; i < times; i++) {
      test();
    }

    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(getTime()).count();
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(getTime()).count();

    std::cout << "Finished " << times << " iterations in " << milliseconds << "ms (" << microseconds << "us)\n\n";

    return milliseconds;
  }

  uint64 Gm_RunBenchmarkTest(const std::function<void()>& test) {
    auto getTime = Gm_CreateTimer();

    test();

    auto time = getTime();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(time).count();
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(time).count();

    std::cout << "Benchmark finished in: " << milliseconds << "ms (" << microseconds << "us)\n";

    return milliseconds;
  }

  void Gm_RunLoopedBenchmarkTest(const std::function<void()>& test, uint32 pause) {
    while (true) {
      Gm_RunBenchmarkTest(test);
      Gm_Sleep(pause);
    }
  }

  void Gm_Sleep(uint32 milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
  }
}