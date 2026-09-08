#include "rtweekend.h"
#include "quaternion.h"
#include "quaternion.h" // 중복 include에도 클래스가 재정의되지 않아야 한다.

#include <stdexcept>
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
  const vec3 p(2, -3, 4);
  near(quaternion().rotate(p), p, "identity는 벡터를 유지");

  // 비단위 축도 허용하며 오른손 법칙을 따른다.
  const auto y90 = quaternion::from_axis_angle(vec3(0, 7, 0), pi / 2);
  near(y90.rotate(vec3(2, 0, 0)), vec3(0, 0, -2), "Y축 90도 회전 방향과 길이");
  near(y90.rotate(vec3(0, 3, 0)), vec3(0, 3, 0), "회전축 위 벡터는 유지");

  const auto arbitrary = quaternion::from_axis_angle(vec3(1, 2, -3), 0.73);
  near(arbitrary.inverse().rotate(arbitrary.rotate(p)), p, "역회전으로 복원");
  near(arbitrary.rotate(p).length(), p.length(), "임의 회전의 길이 보존");
  near(arbitrary.rotate(vec3()), vec3(), "0벡터 회전");

  const auto x90 = quaternion::from_axis_angle(vec3(1, 0, 0), pi / 2);
  const vec3 z(0, 0, 1);
  near((y90 * x90).rotate(z), vec3(0, -1, 0), "합성은 오른쪽 X 회전부터 적용");
  near((x90 * y90).rotate(z), vec3(1, 0, 0), "합성 순서를 바꾸면 다른 결과");
  near((y90 * arbitrary).rotate(p), y90.rotate(arbitrary.rotate(p)),
       "합성과 순차 회전의 일치");

  auto accumulated = quaternion();
  const auto step = quaternion::from_axis_angle(vec3(1, 2, 3), 2 * pi / 1000);
  for (int i = 0; i < 1000; ++i)
    accumulated = step * accumulated;
  near(accumulated.rotate(p), p, "반복 합성으로 한 바퀴 회전");
  near(accumulated.rotate(p).length(), p.length(), "반복 합성 후 길이 보존");

  bool rejected = false;
  try {
    quaternion::from_axis_angle(vec3(), 0.5);
  } catch (const std::invalid_argument&) {
    rejected = true;
  }
  require(rejected, "0벡터 회전축 거부");
}

int main() {
  try {
    tests();
    std::cout << "쿼터니언 테스트를 모두 통과했습니다.\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "쿼터니언 실패: " << error.what() << '\n';
    return 1;
  }
}
