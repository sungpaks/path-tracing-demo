#ifndef QUAD_LIGHT_H
#define QUAD_LIGHT_H

#include "rtweekend.h"
#include "hittable.h"
#include "material.h"
#include <stdexcept>

// 한쪽 면만 발광하는 직사각형. 두 변은 서로 수직이다.
class quad_light : public hittable {
public:
  quad_light(const point3& corner, const vec3& u, const vec3& v, const color& radiance)
      : corner(corner), u(u), v(v), radiance(radiance),
        mat(make_shared<diffuse_light>(radiance, true)) {
    // 외적의 크기는 면적, 방향은 발광 면의 법선이다.
    area = cross(u, v).length();
    if (!(area > 0) || std::abs(dot(u, v)) > 1e-10 * area)
      throw std::invalid_argument("Light needs nonzero perpendicular edges");
    normal = unit_vector(cross(u, v));
  }

  bool hit(const ray& r, interval range, hit_record& rec) const override {
    // 광선과 평면의 교점을 구한다.
    const double denominator = dot(normal, r.direction());
    if (denominator == 0)
      return false;
    const double t = dot(corner - r.origin(), normal) / denominator;
    if (!range.surrounds(t))
      return false;
    // 두 변에 대한 좌표가 [0,1]이면 사각형 내부다.
    const vec3 p = r.at(t) - corner;
    const double a = dot(p, u) / u.length_squared();
    const double b = dot(p, v) / v.length_squared();
    if (a < 0 || a > 1 || b < 0 || b > 1)
      return false;
    rec.t = t;
    rec.p = r.at(t);
    rec.set_face_normal(r, normal);
    rec.mat = mat;
    return true;
  }

  // 이 광원만 사용하는 재질 포인터로 교점을 식별한다.
  bool owns(const hit_record& rec) const { return rec.mat == mat; }

  color sample_direct(const hit_record& rec, const color& albedo, const hittable& world) const {
    // 광원 면적에 대해 균일하게 점을 선택한다.
    const double a = random_double();
    const double b = random_double();
    const point3 target = corner + a * u + b * v; // Q
    const vec3 delta = target - rec.p;            // Q - P
    const double distance2 = delta.length_squared();
    if (!(distance2 > 0))
      return color(0, 0, 0);

    const double distance = std::sqrt(distance2);
    const vec3 wi = delta / distance;
    const double surface_cos = std::fmax(0, dot(rec.normal, wi));
    const double light_cos = std::fmax(0, dot(normal, -wi));
    if (surface_cos == 0 || light_cos == 0 || distance <= 0.001)
      return color(0, 0, 0);

    // 기존 광선과 같은 시작점·하한값을 사용한다.
    // 첫 교점이 이 광원이면 가려지지 않은 것이다.
    hit_record blocker;
    if (!world.hit(ray(rec.p, wi), interval(0.001, infinity), blocker) || !owns(blocker))
      return color(0, 0, 0);
    // Lambertian 반사 × 방출광 × 기하항 ÷ 면적 확률밀도(1/area).
    return (albedo / pi) * radiance * (surface_cos * light_cos * area / distance2);
  }

private:
  point3 corner;
  vec3 u, v, normal;
  color radiance;
  double area;
  shared_ptr<material> mat;
};
#endif
