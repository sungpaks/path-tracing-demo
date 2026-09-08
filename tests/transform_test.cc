#include "transform.h"
#include "transform.h" // 단독 및 중복 include 확인.

#include <limits>
#include <string>

void require(bool ok, const std::string& name) {
  if (!ok)
    throw std::runtime_error(name);
}

void near(double actual, double expected, const std::string& name) {
  require(std::isfinite(actual) && std::abs(actual - expected) < 1e-10, name);
}

void near(const vec3& actual, const vec3& expected, const std::string& name) {
  for (int axis = 0; axis < 3; ++axis)
    near(actual[axis], expected[axis], name + " / 축 " + std::to_string(axis));
}

void tests() {
  const point3 p(2, -3, 4);
  const transform identity;
  near(identity.apply_point(p), p, "identity 점");
  near(identity.apply_vector(p), p, "identity 벡터");
  near(identity.inverse_point(p), p, "identity 역변환 점");
  near(identity.inverse_vector(p), p, "identity 역변환 벡터");
  near(identity.apply_normal(unit_vector(p)), unit_vector(p), "identity 법선");

  const transform tr(vec3(3, -2, 5),
                     quaternion::from_axis_angle(vec3(0, 1, 0), pi / 2),
                     vec3(2, 3, 4));
  near(tr.apply_point(point3(1, 2, 3)), point3(15, 4, 3), "scale → rotation → translation");
  near(tr.apply_vector(vec3(1, 2, 3)), vec3(12, 6, -2), "벡터에는 이동 미적용");
  near(tr.inverse_point(tr.apply_point(p)), p, "점 역변환 복원");
  near(tr.inverse_vector(tr.apply_vector(p)), p, "벡터 역변환 복원");

  const ray parent_ray(point3(7, -1, 2), vec3(2, -3, 5));
  const ray local_ray(tr.inverse_point(parent_ray.origin()),
                      tr.inverse_vector(parent_ray.direction()));
  for (double t : {0.0, 0.25, 3.0})
    near(tr.apply_point(local_ray.at(t)), parent_ray.at(t), "ray의 동일한 t 보존");

  // 법선과 접선의 직교성이 비균일 스케일 및 회전 후에도 유지되어야 한다.
  const vec3 normal = tr.apply_normal(vec3(1, 1, 0));
  near(normal, unit_vector(vec3(0, 1.0 / 3, -0.5)), "역전치 법선의 예상 방향");
  near(normal.length(), 1, "법선 정규화");
  near(dot(normal, tr.apply_vector(vec3(1, -1, 0))), 0, "첫 접선과 수직");
  near(dot(normal, tr.apply_vector(vec3(0, 0, 1))), 0, "두 번째 접선과 수직");

  for (int axis = 0; axis < 3; ++axis) {
    for (double invalid : {0.0, -1.0, std::numeric_limits<double>::infinity(),
                           std::numeric_limits<double>::quiet_NaN()}) {
      vec3 scale(1, 1, 1);
      scale[axis] = invalid;
      bool rejected = false;
      try {
        const transform bad(vec3(), quaternion(), scale);
      } catch (const std::invalid_argument&) {
        rejected = true;
      }
      require(rejected, "각 축의 유효하지 않은 scale 거부");
    }
  }
}

int main() {
  try {
    tests();
    std::cout << "변환 테스트를 모두 통과했습니다.\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "변환 실패: " << error.what() << '\n';
    return 1;
  }
}
