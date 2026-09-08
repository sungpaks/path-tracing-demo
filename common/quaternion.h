#ifndef QUATERNION_H
#define QUATERNION_H

#include <stdexcept>
#include <cmath>

#include "vec3.h"

// 항상 단위 쿼터니언 유지
class quaternion {
public:
  quaternion() : w(1), v(0, 0, 0) {}

  static quaternion from_axis_angle(const vec3& axis, double radians) {
    if (axis.length_squared() == 0)
      throw std::invalid_argument("rotation axis must be nonzero");

    const double half_theta = radians / 2;

    return quaternion(std::cos(half_theta), unit_vector(axis) * std::sin(half_theta));
  }

  quaternion inverse() const { return quaternion(w, -v); }

  vec3 rotate(const vec3& p) const {
    const vec3 t = 2 * cross(v, p);
    return p + w * t + cross(v, t);
  }

  quaternion operator*(const quaternion& rhs) const {
    const double result_w = w * rhs.w - dot(v, rhs.v);
    const vec3 result_v = w * rhs.v + rhs.w * v + cross(v, rhs.v);

    const double length = std::sqrt(result_w * result_w + result_v.length_squared());

    return quaternion(result_w / length, result_v / length);
  }

private:
  double w;
  vec3 v;

  quaternion(double w, const vec3& v) : w(w), v(v) {}
};

#endif
