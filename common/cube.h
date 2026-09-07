#ifndef CUBE_H
#define CUBE_H

#include "rtweekend.h"
#include "hittable.h"

#include <utility>

class cube : public hittable {
public:
  cube(const point3& center, const vec3& size, shared_ptr<material> mat)
      : center(center),
        size(std::fmax(0.0, size.x()), std::fmax(0.0, size.y()), std::fmax(0.0, size.z())),
        mat(mat) {}

  bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
    // 두께가 없으면 패스
    if (size.x() <= 0 || size.y() <= 0 || size.z() <= 0)
      return false;

    // 방향이 없는 ray 패스
    if (r.direction().x() == 0.0 && r.direction().y() == 0.0 && r.direction().z() == 0.0)
      return false;

    const vec3 half_size = size / 2;
    const point3 box_min = center - half_size;
    const point3 box_max = center + half_size;

    double t_enter = -infinity;
    double t_exit = infinity;
    vec3 enter_normal(0, 0, 0);
    vec3 exit_normal(0, 0, 0);

    for (int axis = 0; axis < 3; ++axis) {
      double origin = r.origin()[axis];
      double direction = r.direction()[axis];

      // 현재 axis 축으로의 방향성 없음
      if (direction == 0.0) {
        // 범위 밖이면 박스에 못들어간다.
        if (origin < box_min[axis] || origin > box_max[axis])
          return false;

        // 범위 안이면 이 축이 t에 관여하지 않는다.
        continue;
      }

      double t_near = (box_min[axis] - origin) / direction;
      double t_far = (box_max[axis] - origin) / direction;

      vec3 near_normal(0, 0, 0);
      vec3 far_normal(0, 0, 0);
      near_normal[axis] = -1; // 축 방향에 반대인게 near면 법선
      far_normal[axis] = 1;   // 축 방향에 나란한게 far면 법선

      // 광선 진행방향이 축의 반대방향이면 큰 좌표쪽 면을 먼저 만난다.
      if (t_near > t_far) {
        std::swap(t_near, t_far);
        std::swap(near_normal, far_normal);
      }

      // 가장 늦게 진입하는 축 면이 박스 진입면(진입점)
      if (t_near > t_enter) {
        t_enter = t_near;
        enter_normal = near_normal;
      }

      // 가장 일찍 이탈하는 축 면이 박스 이탈면(이탈점)
      if (t_far < t_exit) {
        t_exit = t_far;
        exit_normal = far_normal;
      }

      if (t_enter > t_exit)
        return false;
    }

    // 허용된 범위 내에 있는 가장 가까운 표면 선택
    double root = t_enter;
    vec3 outward_normal = enter_normal;

    if (!ray_t.surrounds(root)) {
      root = t_exit;
      outward_normal = exit_normal;

      if (!ray_t.surrounds(root))
        return false;
    }

    rec.t = root;
    rec.p = r.at(root);
    rec.set_face_normal(r, outward_normal);
    rec.mat = mat;

    return true;
  }

private:
  point3 center;
  vec3 size;
  shared_ptr<material> mat;
};

#endif
