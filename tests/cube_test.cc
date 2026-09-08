#include "hit_test_support.h"
#include "cube.h"

void tests() {
  using namespace hit_test;
  const auto mat = make_shared<lambertian>(color(0.3, 0.5, 0.7));
  const vec3 shift(4, -2, 7);
  const vec3 size(2, 4, 6);
  const cube object(point3(0, 0, 0), size, mat);
  const cube moved(shift, size, mat);
  const interval range(0.001, infinity);
  common(object, moved, mat, point3(-3, 0, 0), 2, 4, shift);
  for (int axis = 0; axis < 3; ++axis) {
    for (int sign : {-1, 1}) {
      vec3 normal;
      normal[axis] = sign;
      const point3 p = (size[axis] / 2) * normal;
      hit("여섯 면 진입", object, ray(p + 2 * normal, -normal), range,
          2, p, normal, true, mat);
      hit("내부에서 여섯 면 이탈", object, ray(point3(0, 0, 0), normal), range,
          size[axis] / 2, p, -normal, false, mat);
    }
  }
  miss("평행한 축의 범위 밖", object, ray(point3(-3, 3, 0), vec3(1, 0, 0)));
  hit("가장 늦은 축 진입 선택", object, ray(point3(-3, -5, 0), vec3(1, 1, 0)),
      range, 3, point3(0, -2, 0), vec3(0, -1, 0), true, mat);
  miss("축별 구간이 겹치지 않음", object, ray(point3(-3, -10, 0), vec3(1, 1, 0)));
  // 같은 시점에 여러 면에 닿으면 x, y, z 순서로 먼저 선택된 법선을 유지한다.
  hit("모서리에서 x면 우선", object, ray(point3(-3, -4, 0), vec3(1, 1, 0)),
      range, 2, point3(-1, -2, 0), vec3(-1, 0, 0), true, mat);
  hit("꼭짓점에서 x면 우선", object, ray(point3(-3, -4, -5), vec3(1, 1, 1)),
      range, 2, point3(-1, -2, -3), vec3(-1, 0, 0), true, mat);
  hit("경계면과 평행하게 진행", object, ray(point3(-3, 2, 0), vec3(1, 0, 0)),
      range, 2, point3(-1, 2, 0), vec3(-1, 0, 0), true, mat);
  for (int axis = 0; axis < 3; ++axis) {
    for (double length : {0.0, -1.0}) {
      vec3 invalid_size = size;
      invalid_size[axis] = length;
      miss("유효하지 않은 축 길이", cube(point3(0, 0, 0), invalid_size, mat),
           ray(point3(-3, 0, 0), vec3(1, 0, 0)));
    }
  }
}

int main() { return hit_test::run(tests, "직육면체"); }
