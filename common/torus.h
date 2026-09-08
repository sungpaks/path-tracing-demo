#ifndef TORUS_H
#define TORUS_H

#include "hittable.h"
#include "vec3.h"
#include "algebra.h"

class torus : public hittable {
public:
  torus(const point3& center, double major_radius, double minor_radius,
        shared_ptr<material> mat)
      : center(center), major_radius(std::fmax(0, major_radius)),
        minor_radius(std::fmax(0, minor_radius)), mat(mat) {}

  bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
    // 구멍이 있는 일반적인 torus만 다룬다.
    if (!(major_radius > minor_radius && minor_radius > 0))
      return false;

    // 1. 월드 공간 광선을 오브젝트 공간으로
    const ray local_ray(r.origin() - center, r.direction());

    // 2. 허용 범위 안의 가장 가까운 교차점 탐색
    double hit_t;
    if (!_find_intersection(local_ray, ray_t, hit_t))
      return false;

    // 3. 법선 계산 및 월드공간 기준 충돌 정보 저장
    const point3 p = local_ray.at(hit_t);
    const double radial_distance = std::sqrt(p.x() * p.x() + p.z() * p.z()); // rho
    const point3 tube_center(major_radius * p.x() / radial_distance, 0,
                             major_radius * p.z() / radial_distance);

    const vec3 outward_normal = unit_vector(p - tube_center);

    rec.t = hit_t;
    rec.p = r.at(hit_t);
    rec.set_face_normal(r, outward_normal);
    rec.mat = mat;

    return true;
  }

private:
  point3 center;
  double major_radius; /** 중심(회전축)에서 튜브 중심선까지 거리 R */
  double minor_radius; /** 튜브 자체의 반지름 r */
  shared_ptr<material> mat;

  bool _find_intersection(const ray& local_ray, interval ray_t, double& hit_t) const {
    const vec3& o = local_ray.origin();
    const vec3& d = local_ray.direction();

    const double R_squared = major_radius * major_radius;
    const double r_squared = minor_radius * minor_radius;

    // p⋅p + R² - r² = a*t² + b*t + c
    const double a = dot(d, d);
    const double b = 2 * dot(o, d);
    const double c = dot(o, o) + R_squared - r_squared;

    if (a == 0.0)
      return false;

    // rho² = p.x² + p.z² = u*t² + v*t + w
    const double u = d.x() * d.x() + d.z() * d.z();
    const double v = 2 * (o.x() * d.x() + o.z() * d.z());
    const double w = o.x() * o.x() + o.z() * o.z();

    // (x² + y² + z² + R² - r²)² - 4R²(x² + z²) = 0
    // => (a*t² + b*t + c)² - 4R²(u*t² + v*t + w) = 0
    const double A = a * a;
    const double B = 2 * a * b;
    const double C = b * b + 2 * a * c - 4 * R_squared * u;
    const double D = 2 * b * c - 4 * R_squared * v;
    const double E = c * c - 4 * R_squared * w;

    const auto roots = algebra::solve_quartic_equation(A, B, C, D, E);

    // 오름차순으로 반환되므로 첫 유효한 근이 가장 가까운 t임
    for (double t : roots) {
      if (ray_t.surrounds(t)) {
        hit_t = t;
        return true;
      }
    }

    return false;
  }
};

#endif