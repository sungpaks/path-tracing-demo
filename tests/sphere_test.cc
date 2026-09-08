#include "hit_test_support.h"
#include "sphere.h"

void tests() {
  using namespace hit_test;
  const auto mat = make_shared<lambertian>(color(0.3, 0.5, 0.7));
  const vec3 shift(4, -2, 7);
  const sphere object(point3(0, 0, 0), 1, mat);
  const sphere moved(shift, 1, mat);
  const interval range(0.001, infinity);
  common(object, moved, mat, point3(-3, 0, 0), 2, 4, shift);
  hit("중심에서 이탈", object, ray(point3(0, 0, 0), vec3(1, 0, 0)), range,
      1, point3(1, 0, 0), vec3(-1, 0, 0), false, mat);
  // 정확한 접선은 dot == 0이므로 현재 규약상 front_face가 false이다.
  hit("접선의 중근", object, ray(point3(1, 0, -3), vec3(0, 0, 1)), range,
      3, point3(1, 0, 0), vec3(-1, 0, 0), false, mat);
  miss("접선보다 바깥", object, ray(point3(1.01, 0, -3), vec3(0, 0, 1)));
  const vec3 outward(0.6, 0.8, 0);
  hit("비스듬한 구면 법선", object, ray(3 * outward, -outward), range,
      2, outward, outward, true, mat);
  miss("표면에서 바깥으로 출발", object, ray(point3(1, 0, 0), vec3(1, 0, 0)));
  hit("표면에서 안쪽으로 출발", object, ray(point3(1, 0, 0), vec3(-1, 0, 0)),
      range, 2, point3(-1, 0, 0), vec3(1, 0, 0), false, mat);
  for (double radius : {0.0, -1.0}) {
    const sphere invalid(point3(0, 0, 0), radius, mat);
    miss("유효하지 않은 반지름", invalid, ray(point3(-3, 0, 0), vec3(1, 0, 0)));
  }
}

int main() { return hit_test::run(tests, "구"); }
