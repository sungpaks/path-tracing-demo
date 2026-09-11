#ifndef ROOM_SCENE_H
#define ROOM_SCENE_H

#include "rtweekend.h"
#include "camera.h"
#include "cube.h"
#include "cylinder.h"
#include "sphere.h"
#include "torus.h"
#include "transformed_hittable.h"
#include "hittable_list.h"
#include "viewer.h"
#include <string>

enum class room_shape { sphere, cube, cylinder, torus };

inline shared_ptr<hittable> make_room_object(room_shape shape, double x,
                                             shared_ptr<material> mat, bool transformed,
                                             int index) {
  const point3 origin(0, 0, 0);
  shared_ptr<hittable> object;
  quaternion rotation;
  vec3 scale(0.78, 0.78, 0.78);
  switch (shape) {
  case room_shape::sphere:
    object = make_shared<sphere>(origin, 0.85, mat);
    break;
  case room_shape::cube:
    object = make_shared<cube>(origin, vec3(1.5, 1.7, 1.5), mat);
    rotation = quaternion::from_axis_angle(vec3(0, 1, 0), degrees_to_radians(18));
    break;
  case room_shape::cylinder:
    object = make_shared<cylinder>(origin, 0.72, 1.9, mat);
    break;
  case room_shape::torus:
    object = make_shared<torus>(origin, 0.62, 0.23, mat);
    // 입구에서 구멍이 보이도록 토러스를 세운다.
    rotation = quaternion::from_axis_angle(vec3(1, 0, 0), degrees_to_radians(90));
    break;
  }
  if (transformed) {
    const vec3 scales[] = {vec3(0.64, 1.02, 0.74), vec3(0.90, 0.65, 0.78),
                           vec3(0.68, 0.94, 0.82)};
    scale = scales[index];
    const auto tilt =
        quaternion::from_axis_angle(vec3(0.4, 1.0, 0.7), degrees_to_radians(24 + index * 13));
    rotation = tilt * rotation;
  }

  // 변환 후 수직 반경을 구해 물체를 바닥에 배치한다.
  const vec3 q = scale * rotation.inverse().rotate(vec3(0, 1, 0));
  double height = 0;
  switch (shape) {
  case room_shape::sphere:
    height = 0.85 * q.length();
    break;
  case room_shape::cube:
    height = 0.75 * std::abs(q.x()) + 0.85 * std::abs(q.y()) + 0.75 * std::abs(q.z());
    break;
  case room_shape::cylinder:
    height = 0.72 * std::sqrt(q.x() * q.x() + q.z() * q.z()) + 0.95 * std::abs(q.y());
    break;
  case room_shape::torus:
    height = 0.62 * std::sqrt(q.x() * q.x() + q.z() * q.z()) + 0.23 * q.length();
    break;
  }
  const double z = transformed ? (index - 1) * 0.25 : -0.25;
  return make_shared<transformed_hittable>(
      object, transform(vec3(x, height + 0.005, z), rotation, scale));
}

inline int run_room_scene(room_shape shape, int argc, char* argv[], bool transformed = false) {
  bool offline = false;
  bool preview = false;
  bool nee = true;
  for (int i = 1; i < argc; ++i) {
    const std::string arg(argv[i]);
    if (arg == "--offline")
      offline = true;
    else if (arg == "--no-nee")
      nee = false;
    else if (arg == "--preview")
      preview = true;
    else {
      std::cerr << "Usage: " << argv[0] << " [--offline] [--preview] [--no-nee]\n";
      return 1;
    }
  }

  hittable_list world;
  const auto white = make_shared<lambertian>(color(0.73, 0.73, 0.73));
  const auto red = make_shared<lambertian>(color(0.65, 0.045, 0.035));
  const auto green = make_shared<lambertian>(color(0.045, 0.55, 0.07));

  // 내부 범위: x,z는 [-3,3], y는 [0,5]. +Z 방향은 열린 입구다.
  world.add(make_shared<cube>(point3(-3.1, 2.5, 0), vec3(0.2, 5.4, 6.4), red));
  world.add(make_shared<cube>(point3(3.1, 2.5, 0), vec3(0.2, 5.4, 6.4), green));
  world.add(make_shared<cube>(point3(0, 2.5, -3.1), vec3(6.4, 5.4, 0.2), white));
  world.add(make_shared<cube>(point3(0, 5.1, 0), vec3(6.4, 0.2, 6.4), white));
  world.add(
      make_shared<cube>(point3(0, -0.1, 0), vec3(6.4, 0.2, 6.4), make_shared<lambertian>(color(0.72, 0.72, 0.72))));

  // 천장 아래 사각 광원. 하늘빛은 사용하지 않는다.
  const auto ceiling_light = make_shared<quad_light>(point3(-1.1, 4.94, -1.2), vec3(2.2, 0, 0),
                                                     vec3(0, 0, 1.8), color(8, 8, 8));
  world.add(ceiling_light);

  // 왼쪽은 유리, 가운데는 무광, 오른쪽은 금속이다.
  world.add(make_room_object(
      shape, -1.85, make_shared<dielectric>(1.5, color(0.015, 0.008, 0.003)), transformed, 0));
  world.add(make_room_object(shape, 0, make_shared<lambertian>(color(0.73, 0.69, 0.55)),
                             transformed, 1));
  world.add(make_room_object(shape, 1.85, make_shared<metal>(color(0.88, 0.82, 0.70), 0.03),
                             transformed, 2));

  camera cam;
  cam.direct_light = ceiling_light;
  cam.enable_nee = nee;
  std::clog << "Lambertian NEE: " << (nee ? "on" : "off") << '\n';
  cam.aspect_ratio = 1;
  cam.image_width = preview ? 160 : 400;
  cam.samples_per_pixel = preview ? 256 : 1024;
  cam.max_depth = 50;
  cam.EYE = point3(0, 2.5, 9);
  cam.AT = point3(0, 2.5, 0);
  cam.vfov = 53;
  cam.focus_dist = 9;
  cam.defocus_angle = 0;
  cam.use_sky_background = false;

  try {
    if (offline) {
      cam.reset_accumulation();
      thread_pool pool(4);
      for (int i = 0; i < cam.samples_per_pixel; ++i)
        cam.render_pass_parallel(world, pool);
      std::cout << "P3\n" << cam.image_width << ' ' << cam.height() << "\n255\n";
      for (int y = 0; y < cam.height(); ++y)
        for (int x = 0; x < cam.image_width; ++x)
          write_color(std::cout, cam.averaged_color(x, y));
    } else {
      viewer app;
      app.run(cam, world);
    }
  } catch (const std::exception& error) {
    std::cerr << "Room render error: " << error.what() << '\n';
    return 1;
  }
  return 0;
}

#endif
