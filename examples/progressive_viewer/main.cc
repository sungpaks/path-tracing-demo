#include "rtweekend.h"
#include "camera.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "cube.h"
#include "cylinder.h"
#include "torus.h"
#include "transformed_hittable.h"
#include "viewer.h"

#include <algorithm>

#include <array>
#include <random>
#include <string>

// 하나의 판 위에 체크무늬와 색 띠를 입혀 굴절 전후의 패턴을 비교한다.
class reference_pattern : public material {
public:
  bool scatter(const ray& r_in, const hit_record& rec, color& attenuation,
               ray& scattered) const override {
    const double tile_size = 0.45;
    const int x = static_cast<int>(std::floor(rec.p.x() / tile_size));
    const int z = static_cast<int>(std::floor(rec.p.z() / tile_size));
    color albedo = (x + z) % 2 == 0 ? color(0.78, 0.80, 0.83) : color(0.12, 0.15, 0.20);

    if (std::abs(rec.p.x() + 3.0) < 0.09)
      albedo = color(0.80, 0.12, 0.05);
    else if (std::abs(rec.p.x()) < 0.09)
      albedo = color(0.04, 0.45, 0.70);
    else if (std::abs(rec.p.x() - 3.0) < 0.09)
      albedo = color(0.80, 0.55, 0.04);

    return lambertian(albedo).scatter(r_in, rec, attenuation, scattered);
  }
};

int main(int argc, char* argv[]) {
  const bool preview = argc > 1 && std::string(argv[1]) == "--preview";
  hittable_list world;

  // 시드를 바꾸면 각 행의 색과 도형의 크기, 위치가 바뀐다.
  std::mt19937 generator(20260908);
  // 변환 전용 난수열로 원본 씬의 색·크기·위치를 유지한다.
  std::mt19937 transform_generator(20260909);
  std::uniform_real_distribution<double> angle_distribution(20, 70);
  std::uniform_real_distribution<double> axis_distribution(-1, 1);
  const auto random_axis = [&]() {
    // 단위 구 내부에서 균일하게 뽑은 뒤 정규화해 방향 편향을 피한다.
    while (true) {
      const double x = axis_distribution(transform_generator);
      const double y = axis_distribution(transform_generator);
      const double z = axis_distribution(transform_generator);
      const vec3 axis(x, y, z);
      const double length_squared = axis.length_squared();
      if (length_squared > 1e-160 && length_squared <= 1)
        return axis / std::sqrt(length_squared);
    }
  };
  std::uniform_real_distribution<double> short_scale(0.65, 0.75);
  std::uniform_real_distribution<double> middle_scale(0.90, 1.00);
  std::uniform_real_distribution<double> long_scale(1.15, 1.25);
  std::uniform_real_distribution<double> size_distribution(1.0, 1.5);
  std::uniform_real_distribution<double> offset_distribution(-0.25, 0.25);
  std::uniform_real_distribution<double> diffuse_distribution(0.12, 0.75);
  std::uniform_real_distribution<double> metal_distribution(0.55, 0.95);
  std::uniform_real_distribution<double> absorption_distribution(0.03, 0.40);

  const color diffuse_albedo(diffuse_distribution(generator), diffuse_distribution(generator),
                             diffuse_distribution(generator));
  const color metal_albedo(metal_distribution(generator), metal_distribution(generator),
                           metal_distribution(generator));
  // absorption은 색의 투과율이 아니라 단위 거리당 흡수 계수다.
  const color absorption(absorption_distribution(generator), absorption_distribution(generator),
                         absorption_distribution(generator));

  const std::array<shared_ptr<material>, 3> materials = {
      {make_shared<lambertian>(diffuse_albedo), make_shared<metal>(metal_albedo, 0.03),
       make_shared<dielectric>(1.5, absorption)}};

  // 열은 구·큐브·원기둥·토러스, 행은 diffuse·metal·dielectric 순서다.
  enum primitive { sphere_shape, cube_shape, cylinder_shape, torus_shape };

  // XZ 평면의 4×3 그리드. 각 셀은 3×3이며 도형의 중심 높이는 같다.
  const int columns = 4;
  const int rows = 3;
  const double cell_size = 3.0;

  for (int cell = 0; cell < columns * rows; ++cell) {
    const double size = size_distribution(generator);
    const double half_size = size / 2;
    const point3 cell_center((cell % columns - (columns - 1) / 2.0) * cell_size, 0,
                             (cell / columns - (rows - 1) / 2.0) * cell_size);

    // 원본과 같은 위치 오프셋을 사용한다.
    vec3 offset;
    do {
      offset = vec3(offset_distribution(generator), 0, offset_distribution(generator));
    } while (offset.length_squared() > 1.0 ||
             std::abs(offset.x()) + half_size > cell_size / 2 ||
             std::abs(offset.z()) + half_size > cell_size / 2);

    const point3 center = cell_center + offset;
    const auto& mat = materials[cell / columns];

    // size는 전체 크기. 구와 원기둥은 지름, 토러스는 바깥 지름을 맞춘다.
    shared_ptr<hittable> object;
    const point3 local_center(0, 0, 0);
    switch (static_cast<primitive>(cell % columns)) {
    case sphere_shape:
      object = make_shared<sphere>(local_center, half_size, mat);
      break;
    case cube_shape:
      object = make_shared<cube>(local_center, vec3(size, size, size), mat);
      break;
    case cylinder_shape:
      object = make_shared<cylinder>(local_center, half_size, size, mat);
      break;
    case torus_shape:
      object = make_shared<torus>(local_center, size * 0.35, size * 0.15, mat);
      break;
    }

    // 축마다 서로 다른 범위에서 뽑아 비균일 스케일이 확실히 드러나게 한다.
    std::array<double, 3> scales = {{short_scale(transform_generator),
                                     middle_scale(transform_generator),
                                     long_scale(transform_generator)}};
    std::shuffle(scales.begin(), scales.end(), transform_generator);
    const vec3 scale(scales[0], scales[1], scales[2]);

    const vec3 axis = random_axis();
    const double angle = degrees_to_radians(angle_distribution(transform_generator));
    const auto rotation = quaternion::from_axis_angle(axis, angle);
    world.add(make_shared<transformed_hittable>(object, transform(center, rotation, scale)));
  }

  // 유한한 기준 판만 두어 가장자리 밖으로 기존 하늘 배경이 보이게 한다.
  world.add(make_shared<cube>(point3(0, -1.45, 0), vec3(13, 0.12, 10),
                              make_shared<reference_pattern>()));

  // 화면 밖의 높은 위치에 둔 판이 금속과 유리 표면에 반사된다.
  world.add(make_shared<cube>(point3(-4, 9, -10), vec3(5, 7, 0.15),
                              make_shared<lambertian>(color(0.92, 0.92, 0.92))));
  world.add(make_shared<cube>(point3(4, 9, -10), vec3(4, 7, 0.15),
                              make_shared<lambertian>(color(0.04, 0.40, 0.65))));

  camera cam;
  cam.aspect_ratio = 4.0 / 3.0;
  cam.image_width = 240; // preview ? 320 : 800;
  cam.samples_per_pixel = preview ? 24 : 150;
  cam.max_depth = 20;
  cam.vfov = 38;
  cam.EYE = point3(2, 15, 11);
  cam.AT = point3(0, -0.5, 0);
  cam.UP = vec3(0, 1, 0);
  cam.defocus_angle = 0;
  cam.focus_dist = (cam.EYE - cam.AT).length();

  // cam.render(world);
  try {
    viewer app;
    app.run(cam, world);
  } catch (const std::exception& error) {
    std::cerr << "viewer error: " << error.what() << '\n';
    return 1;
  }
}
