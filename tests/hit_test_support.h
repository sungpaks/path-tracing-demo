#ifndef HIT_TEST_SUPPORT_H
#define HIT_TEST_SUPPORT_H

#include "rtweekend.h"
#include "material.h"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace hit_test {
inline void require(bool ok, const std::string& name) {
  if (!ok)
    throw std::runtime_error(name);
}

inline void near(double actual, double expected, const std::string& name,
                 double tolerance = 1e-7) {
  require(std::isfinite(actual) &&
              std::abs(actual - expected) <= tolerance * std::max(1.0, std::abs(expected)),
          name + " (실제: " + std::to_string(actual) + ", 기대: " +
              std::to_string(expected) + ")");
}

inline void near(const vec3& actual, const vec3& expected, const std::string& name) {
  for (int axis = 0; axis < 3; ++axis)
    near(actual[axis], expected[axis], name + " / 축 " + std::to_string(axis));
}

inline void hit(const std::string& name, const hittable& object, const ray& r,
                interval range, double t, const point3& p, const vec3& normal,
                bool front_face, const shared_ptr<material>& mat) {
  hit_record rec;
  require(object.hit(r, range, rec), name + ": 충돌해야 함");
  near(rec.t, t, name + ": 교차 시점");
  near(rec.p, p, name + ": 충돌 위치");
  near(rec.normal, normal, name + ": 정렬된 법선");
  near(rec.normal.length(), 1, name + ": 단위 법선");
  require(dot(r.direction(), rec.normal) <= 1e-7, name + ": 법선은 광선 반대 방향");
  require(rec.front_face == front_face, name + ": 앞뒷면 구분");
  require(rec.mat == mat, name + ": 재질 보존");
}

inline void miss(const std::string& name, const hittable& object, const ray& r,
                 interval range = interval(0.001, infinity)) {
  hit_record rec;
  require(!object.hit(r, range, rec), name + ": 충돌하지 않아야 함");
}

// x축을 따라 두 표면을 관통하는 광선으로 공통 규약을 검증한다.
inline void common(const hittable& object, const hittable& moved,
                   const shared_ptr<material>& mat, const point3& origin,
                   double enter, double exit, const vec3& translation) {
  const ray r(origin, vec3(1, 0, 0));
  const interval range(0.001, infinity);
  hit("바깥에서 진입", object, r, range, enter, r.at(enter), vec3(-1, 0, 0), true, mat);
  hit("가까운 근을 제외하고 이탈점 선택", object, r, interval(enter + 0.1, exit + 0.1),
      exit, r.at(exit), vec3(-1, 0, 0), false, mat);
  miss("진입 전 검색 종료", object, r, interval(0.001, enter - 0.1));
  miss("표면 사이만 검색", object, r, interval(enter + 0.1, exit - 0.1));
  miss("구간 최댓값과 같은 근 제외", object, r, interval(0.001, enter));
  miss("구간 최솟값과 같은 근 제외", object, r, interval(enter, exit - 0.1));
  miss("빈 검색 구간", object, r, interval(enter, enter));
  miss("반대 방향으로 진행", object, ray(origin, vec3(-1, 0, 0)));
  miss("0벡터 방향", object, ray(point3(0, 0, 0), vec3(0, 0, 0)));
  hit("방향 길이가 두 배이면 t는 절반", object, ray(origin, vec3(2, 0, 0)), range,
      enter / 2, r.at(enter), vec3(-1, 0, 0), true, mat);
  hit("도형과 광선을 함께 이동", moved, ray(origin + translation, vec3(1, 0, 0)),
      range, enter, r.at(enter) + translation, vec3(-1, 0, 0), true, mat);
}

inline int run(void (*tests)(), const char* name) {
  try {
    tests();
    std::cout << name << " 충돌 테스트를 모두 통과했습니다.\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << name << " 실패: " << error.what() << '\n';
    return 1;
  }
}
} // namespace hit_test
#endif
