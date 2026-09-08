#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "rtweekend.h"
#include "quaternion.h"

#include <stdexcept>

class transform {
public:
  transform(const vec3& translation = vec3(0, 0, 0), const quaternion& rotation = quaternion(),
            const vec3& scale = vec3(1, 1, 1))
      : translation(translation), rotation(rotation), scale(scale) {
    for (int i = 0; i < 3; ++i) {
      if (!std::isfinite(scale[i]) || scale[i] <= 0)
        throw std::invalid_argument("scale must be finite and positive");
    }
  }

  point3 apply_point(const point3& p) const { return rotation.rotate(scale * p) + translation; }
  vec3 apply_vector(const vec3& v) const { return rotation.rotate(scale * v); }

  point3 inverse_point(const point3& p) const {
    return divide_by_scale(rotation.inverse().rotate(p - translation));
  }
  vec3 inverse_vector(const vec3& v) const {
    return divide_by_scale(rotation.inverse().rotate(v));
  }

  vec3 apply_normal(const vec3& normal) const {
    return unit_vector(rotation.rotate(divide_by_scale(normal)));
  }

private:
  vec3 translation;
  quaternion rotation;
  vec3 scale;

  vec3 divide_by_scale(const vec3& v) const {
    return vec3(v.x() / scale.x(), v.y() / scale.y(), v.z() / scale.z());
  }
};

#endif