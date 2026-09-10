#ifndef INTERVAL_H
#define INTERVAL_H

#include "rt_utils.h"

class interval {
public:
  double min, max;

  interval() : min(+infinity), max(-infinity) {} // Default interval = empty
  interval(double min, double max) : min(min), max(max) {}

  double size() const { return max - min; }

  bool contains(double x) const { return min <= x && x <= max; }

  /**
   * x가 지정한 구간에 속하나? min < x < max
   */
  bool surrounds(double x) const { return min < x && x < max; }

  /**
   * x가 지정한 구간 min < x < max 로 들어오도록 한다. 
   */
  double clamp(double x) const {
    if (x < min)
      return min;
    if (x > max)
      return max;
    return x;
  }

  static const interval empty, universe;
};

const interval interval::empty = interval(+infinity, -infinity);
const interval interval::universe = interval(-infinity, +infinity);

#endif
