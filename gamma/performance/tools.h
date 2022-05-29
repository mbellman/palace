#pragma once

#include "system/type_aliases.h"

namespace Gamma {
  template<uint32 size, typename T>
  class Averager {
  public:
    Averager() {
      values = new T[size];
    }

    ~Averager() {
      delete[] values;
    }

    void add(T value) {
      values[index++ % size] = value;
    }

    T average() {
      T sum = (T)0;
      uint32 total = index > size ? size : index;

      for (uint32 i = 0; i < total; i++) {
        sum += values[i];
      }

      return (T)(sum / (float)total);
    }

    T high() {
      T highest = (T)0;
      uint32 total = index > size ? size : index;

      for (uint32 i = 0; i < total; i++) {
        if (values[i] > highest) {
          highest = values[i];
        }
      }

      return highest;
    }

    T low() {
      T lowest = (T)values[0];
      uint32 total = index > size ? size : index;

      for (uint32 i = 0; i < total; i++) {
        if (values[i] < lowest) {
          lowest = values[i];
        }
      }

      return lowest;
    }

  private:
    T* values = nullptr;
    uint32 index = 0;
  };
}