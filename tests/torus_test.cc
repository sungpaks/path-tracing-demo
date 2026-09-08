#include "hit_test_support.h"
#include "torus.h"

void tests() {
  using namespace hit_test;
  const auto mat = make_shared<lambertian>(color(0.3, 0.5, 0.7));
  const vec3 shift(4, -2, 7);
  const torus object(point3(0, 0, 0), 2, 0.5, mat);
  const torus moved(shift, 2, 0.5, mat);
  const interval range(0.001, infinity);
  common(object, moved, mat, point3(-4, 0, 0), 1.5, 2.5, shift);
  const ray horizontal(point3(-4, 0, 0), vec3(1, 0, 0));
  const double roots[] = {1.5, 2.5, 5.5, 6.5};
  for (int i = 0; i < 4; ++i) {
    hit("구간에 따라 네 교차점 선택", object, horizontal,
        interval(roots[i] - 0.25, roots[i] + 0.25), roots[i],
        point3(-4 + roots[i], 0, 0), vec3(-1, 0, 0), i % 2 == 0, mat);
  }
  miss("마지막 교차점보다 뒤에서 검색", object, horizontal, interval(6.6, infinity));
  miss("중앙 구멍을 수직 통과", object, ray(point3(0, 3, 0), vec3(0, -1, 0)));
  hit("튜브를 수직 관통", object, ray(point3(2, 3, 0), vec3(0, -1, 0)), range,
      2.5, point3(2, 0.5, 0), vec3(0, 1, 0), true, mat);
  hit("튜브 내부에서 이탈", object, ray(point3(2, 0, 0), vec3(1, 0, 0)), range,
      0.5, point3(2.5, 0, 0), vec3(-1, 0, 0), false, mat);
  hit("구멍은 내부가 아니며 안쪽 표면으로 진입", object,
      ray(point3(0, 0, 0), vec3(1, 0, 0)), range,
      1.5, point3(1.5, 0, 0), vec3(-1, 0, 0), true, mat);
  hit("바깥 표면 접선", object, ray(point3(2.5, 0, -3), vec3(0, 0, 1)), range,
      3, point3(2.5, 0, 0), vec3(-1, 0, 0), false, mat);
  miss("접선 바로 바깥", object, ray(point3(2.5001, 0, -3), vec3(0, 0, 1)));
  const double x = 2.4999;
  const double z = -std::sqrt(2.5 * 2.5 - x * x);
  hit("접선 바로 안쪽", object, ray(point3(x, 0, -3), vec3(0, 0, 1)), range,
      3 + z, point3(x, 0, z), vec3(x / 2.5, 0, z / 2.5), true, mat);
  // 튜브의 3:4:5 단면으로 축에 정렬되지 않은 법선을 독립적으로 계산한다.
  hit("튜브 단면의 비스듬한 법선", object,
      ray(point3(2.3, 2, 0), vec3(0, -1, 0)), range,
      1.6, point3(2.3, 0.4, 0), vec3(0.6, 0.8, 0), true, mat);
  const double invalid[][2] = {{0, 0.5}, {-1, 0.5}, {2, 0}, {2, -1}, {1, 1}, {0.5, 1}};
  for (const auto& radii : invalid) {
    miss("지원하지 않는 반지름 조합", torus(point3(0, 0, 0), radii[0], radii[1], mat),
         horizontal);
  }
}

int main() { return hit_test::run(tests, "토러스"); }
