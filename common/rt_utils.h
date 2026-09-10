#ifndef RT_UTILS_H
#define RT_UTILS_H

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include <atomic>
#include <random>

// C++ Std Usings

using std::make_shared;
using std::shared_ptr;

inline std::mt19937& random_generator() {
  static std::atomic<unsigned int> next_id{0};

  thread_local std::mt19937 generator([]() {
    const unsigned int id = next_id.fetch_add(1, std::memory_order_relaxed);

    std::seed_seq seed{20260910u, id};
    return std::mt19937(seed);
  }());

  return generator;
}

// Constants

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility Functions

inline double degrees_to_radians(double degrees) {
  return degrees * pi / 180.0;
}

inline double random_double() {
  // [0, 1)
  return std::generate_canonical<double, 53>(random_generator());
}

inline double random_double(double min, double max) {
  // [min, max)
  return min + (max - min) * random_double();
}

#endif
