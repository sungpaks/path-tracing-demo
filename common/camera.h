#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"
#include "material.h"

class camera {
public:
  // 카메라 파라미터 (public)
  double aspect_ratio = 1.0;  // (ideal) 종횡비 aspect ratio
  int image_width = 100;      // 렌더링되는 이미지의 가로 픽셀 수
  int samples_per_pixel = 10; // 한 픽셀에 대해, 랜덤 샘플링하는 수
  int max_depth = 10;         // 최대 bounce 횟수

  void render(const hittable& world) {
    initialize();

    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    for (int j = 0; j < image_height; j++) {
      std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
      for (int i = 0; i < image_width; i++) {
        color pixel_color(0, 0, 0);
        for (int sample = 0; sample < samples_per_pixel; sample++) {
          ray r = get_ray(i, j);
          pixel_color += ray_color(r, max_depth, world);
        }
        write_color(std::cout, pixel_samples_scale * pixel_color);
      }
    }

    std::clog << "\rDone.                 \n";
  }

private:
  // 카메라 내부 변수 (private)
  int image_height;           // 렌더링된 이미지의 세로축 픽셀 수
  double pixel_samples_scale; // 색상 scale factor
  point3 center;              // 카메라 중심
  point3 pixel00_loc;         // 좌상단(가장 처음) 픽셀의 위치
  vec3 pixel_delta_u;         // pixel 간 width축(u축) 거리
  vec3 pixel_delta_v;         // pixel 간 height축(v축) 거리

  void initialize() {
    image_height = int(image_width / aspect_ratio);
    image_height = (image_height < 1) ? 1 : image_height;

    pixel_samples_scale = 1.0 / samples_per_pixel;

    center = point3(0, 0, 0);

    // Viewport의 Dimension
    auto focal_length = 1.0;
    auto viewport_height = 2.0;
    auto viewport_width = viewport_height * (double(image_width) / image_height);

    // Vu, Vv (Image Plane인 Viewport를 따라 좌상단부터 우측아래로 내려가는)
    auto viewport_u = vec3(viewport_width, 0, 0);
    auto viewport_v = vec3(0, -viewport_height, 0);

    // u,v축을 따라 픽셀 간을 이동하는 델타 벡터
    pixel_delta_u = viewport_u / image_width;
    pixel_delta_v = viewport_v / image_height;

    // 좌상단 픽셀 위치 계산
    auto viewport_upper_left =
        center - vec3(0, 0, focal_length) - viewport_u / 2 - viewport_v / 2;
    pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);
  }

  ray get_ray(int i, int j) const {
    // 어떤 위치 i, j에서, origin에서 랜덤 샘플된 지점으로 향하는 ray를 얻는다.
    auto offset = sample_square();
    auto pixel_sample =
        pixel00_loc + ((i + offset.x()) * pixel_delta_u) + ((j + offset.y()) * pixel_delta_v);

    auto ray_origin = center;
    auto ray_direction = pixel_sample - ray_origin;

    return ray(ray_origin, ray_direction);
  }

  vec3 sample_square() const {
    // [-0.5, -0.5] ~ [0.5, 0.5] unit square에서의 랜덤 지점 뽑기
    return vec3(random_double() - 0.5, random_double() - 0.5, 0);
  }

  color ray_color(const ray& r, int depth, const hittable& world) const {
    if (depth <= 0)
      return color(0, 0, 0); // ray bounce limit 넘어가면 빛 없음으로 처리
    hit_record rec;

    if (world.hit(r, interval(0.001, infinity), rec)) {
      ray scattered;
      color attenuation;
      if (rec.mat->scatter(r, rec, attenuation, scattered))
        return attenuation * ray_color(scattered, depth - 1, world);
      return color(0, 0, 0);
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto a = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
  }
};

#endif