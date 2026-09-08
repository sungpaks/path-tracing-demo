#include "hit_test_support.h"
#include "cylinder.h"

void tests() {
  using namespace hit_test;
  const auto mat = make_shared<lambertian>(color(0.3, 0.5, 0.7));
  const vec3 shift(4, -2, 7);
  const cylinder object(point3(0, 0, 0), 1, 2, mat);
  const cylinder moved(shift, 1, 2, mat);
  const interval range(0.001, infinity);
  common(object, moved, mat, point3(-3, 0, 0), 2, 4, shift);
  hit("내부에서 옆면 이탈", object, ray(point3(0, 0, 0), vec3(1, 0, 0)), range,
      1, point3(1, 0, 0), vec3(-1, 0, 0), false, mat);
  for (int sign : {-1, 1}) {
    const vec3 normal(0, sign, 0);
    hit("위아래 뚜껑 진입", object, ray(3 * normal, -normal), range,
        2, normal, normal, true, mat);
    hit("내부에서 뚜껑 이탈", object, ray(point3(0, 0, 0), normal), range,
        1, normal, -normal, false, mat);
  }
  miss("옆면 교차점이 높이 밖", object, ray(point3(3, 2, 0), vec3(-1, 0, 0)));
  miss("뚜껑 평면 교차점이 원 밖", object, ray(point3(2, 3, 0), vec3(0, -1, 0)));
  const ray diagonal(point3(2, 3, 0), vec3(-1, -1, 0));
  hit("먼 옆면보다 가까운 뚜껑 선택", object, diagonal, range,
      2, point3(0, 1, 0), vec3(0, 1, 0), true, mat);
  hit("높이를 벗어난 첫 옆면 근은 버리고 두 번째 근 선택", object, diagonal,
      interval(2.1, infinity), 3, point3(-1, 0, 0), vec3(1, 0, 0), false, mat);
  hit("옆면과 뚜껑 테두리는 옆면 우선", object,
      ray(point3(2, 2, 0), vec3(-1, -1, 0)), range,
      1, point3(1, 1, 0), vec3(1, 0, 0), true, mat);
  hit("옆면 접선", object, ray(point3(1, 0, -3), vec3(0, 0, 1)), range,
      3, point3(1, 0, 0), vec3(-1, 0, 0), false, mat);
  const vec3 normal(0.6, 0, 0.8);
  hit("비스듬한 옆면 법선", object, ray(3 * normal, -normal), range,
      2, normal, normal, true, mat);
  for (double length : {0.0, -1.0}) {
    miss("유효하지 않은 반지름", cylinder(point3(0, 0, 0), length, 2, mat),
         ray(point3(-3, 0, 0), vec3(1, 0, 0)));
    miss("유효하지 않은 높이", cylinder(point3(0, 0, 0), 1, length, mat),
         ray(point3(-3, 0, 0), vec3(1, 0, 0)));
  }
}

int main() { return hit_test::run(tests, "원기둥"); }
