#include "color.h"
#include "vec3.h"
#include "ray.h"

#include <iostream>

color ray_color(const ray& r) {
  vec3 unit_direction = unit_vector(r.direction());
  auto t = 0.5 * (unit_direction.y() + 1.0);
  return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

int main() {
  // Image
  auto aspect_ratio = 16.0 / 9.0;
  int image_width = 400;
  int image_height = int(image_width / aspect_ratio);
  image_height = (image_height < 1) ? 1 : image_height;

  // Camera
  auto focal_length = 1.0;
  auto viewport_height = 2.0;
  auto viewport_width = viewport_height * (double(image_width) / image_height);
  auto camera_center = point3(0, 0, 0);

  // Vu, Vv (Image Plane인 Viewport를 따라 좌상단부터 우측아래로 내려가는)
  auto viewport_u = vec3(viewport_width, 0, 0);
  auto viewport_v = vec3(0, -viewport_height, 0);

  // u,v축을 따라 픽셀 간을 이동하는 델타 벡터
  auto pixel_delta_u = viewport_u / image_width;
  auto pixel_delta_v = viewport_v / image_height;

  // 좌상단 픽셀 위치
  auto viewport_upper_left =
      camera_center - vec3(0, 0, focal_length) - viewport_u / 2 - viewport_v / 2;
  auto pixel00_location = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

  // Render

  std::cout << "P3\n" << image_width << " " << image_height << "\n255\n";

  for (int i = 0; i < image_height; i++) {
    std::clog << "\rScanlines remaining: " << (image_height - i) << ' ' << std::flush;
    for (int j = 0; j < image_width; j++) {
      auto pixel_center = pixel00_location + (j * pixel_delta_u) + (i * pixel_delta_v);
      auto ray_direciton = pixel_center - camera_center;
      ray r(camera_center, ray_direciton);

      color pixel_color = ray_color(r);
      write_color(std::cout, pixel_color);
    }
  }
  std::clog << "\rDone.                 \n";
}