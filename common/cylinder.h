#ifndef CYLINDER_H
#define CYLINDER_H

#include "rtweekend.h"
#include "hittable.h"

class cylinder : public hittable {
public:
  cylinder(const point3& center, double radius, double height, shared_ptr<material> mat)
      : center(center), radius(std::fmax(0, radius)), height(std::fmax(0, height)), mat(mat) {}

  bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
    if (radius <= 0 || height <= 0)
      return false;

    const ray local_ray(r.origin() - center, r.direction());
    double closest_t = ray_t.max;
    vec3 outward_normal;

    const bool hit_side = _hit_side_surface(local_ray, ray_t, closest_t, outward_normal);
    const bool hit_cap =
        _hit_caps(local_ray, interval(ray_t.min, closest_t), closest_t, outward_normal);

    if (!hit_side && !hit_cap)
      return false;

    rec.t = closest_t;
    rec.p = r.at(closest_t);
    rec.set_face_normal(r, outward_normal);
    rec.mat = mat;
    return true;
  }

private:
  bool _hit_side_surface(const ray& local_ray, interval ray_t, double& hit_t,
                         vec3& outward_normal) const {
    const vec3& o = local_ray.origin();
    const vec3& d = local_ray.direction();
    const double half_height = height / 2;
    const double radius_squared = radius * radius;

    // 옆면:  x² + z² = radius²
    const double a = d.x() * d.x() + d.z() * d.z();
    const double half_b = o.x() * d.x() + o.z() * d.z();
    const double c = o.x() * o.x() + o.z() * o.z() - radius_squared;

    // 원기둥축과 평행하지 않은 광선만 옆면검사.
    if (a > 0.0) {
      const double discriminant = half_b * half_b - a * c;
      if (discriminant >= 0.0) {
        const double sqrt_discriminant = std::sqrt(discriminant);
        // 이차방정식의 두 근 (옆면과 만나는 두 t). root[0]이 먼저 만난 시점, root[1]이 나중
        const double roots[2] = {(-half_b - sqrt_discriminant) / a,
                                 (-half_b + sqrt_discriminant) / a};

        for (double t : roots) {
          if (!ray_t.surrounds(t))
            continue;

          const vec3 local_hit = o + t * d;

          // 원기둥의 높이를 벗어남
          if (local_hit.y() < -half_height || local_hit.y() > half_height)
            continue;

          hit_t = t;
          outward_normal = vec3(local_hit.x() / radius, 0, local_hit.z() / radius);
          return true;
        }
      }
    }
    return false;
  }

  bool _hit_caps(const ray& local_ray, interval ray_t, double& hit_t,
                 vec3& outward_normal) const {
    const vec3& o = local_ray.origin();
    const vec3& d = local_ray.direction();
    const double half_height = height / 2;
    const double radius_squared = radius * radius;
    bool hit_anything = false;

    // 뚜껑과 평행하지 않은 광선만 검사
    if (d.y() != 0.0) {
      // 아래뚜껑(-1), 윗뚜껑(1)
      for (int sign : {-1, 1}) {
        const double cap_y = sign * half_height;  // 아래/위 뚜껑 높이
        const double t = (cap_y - o.y()) / d.y(); // 뚜껑 평면과 만나는 곳
        if (!ray_t.surrounds(t))
          continue;

        const vec3 local_hit = o + t * d;
        const double distance_squared =
            local_hit.x() * local_hit.x() + local_hit.z() * local_hit.z();

        // 평면과 만난 점이 뚜껑의 원 안에 있어야 함
        if (distance_squared > radius_squared)
          continue;

        hit_t = t;
        outward_normal = vec3(0, sign, 0);
        ray_t.max = t;
        hit_anything = true;
      }
    }

    return hit_anything;
  }

  point3 center;
  double radius;
  double height;
  shared_ptr<material> mat;
};

#endif
