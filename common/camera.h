#ifndef CAMERA_H
#define CAMERA_H

#include "rtweekend.h"
#include "hittable.h"
#include "material.h"

#include <vector>

class camera {
public:
  // 카메라 파라미터 (public)
  double aspect_ratio = 1.0;    // (ideal) 종횡비 aspect ratio
  int image_width = 100;        // 렌더링되는 이미지의 가로 픽셀 수
  int samples_per_pixel = 10;   // 한 픽셀에 대해, 랜덤 샘플링하는 수
  int max_depth = 10;           // 최대 bounce 횟수
  double vfov = 90;             // Vertical 시야각
  point3 EYE = point3(0, 0, 0); // EYE.
  point3 AT = point3(0, 0, -1); // AT.
  vec3 UP = vec3(0, 1, 0);      // UP.
  double defocus_angle = 0;     // defocus blur를 위한 disk 크기 결정
  double focus_dist = 10;       // EYE에서 perfect focus plane까지의 거리

  /** deprecated: (Bucket Rendering, Non-progressive) */
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

  int height() const { return image_height; }
  int sample_count() const { return accumulated_samples; }

  /** 누적 샘플 초기화 */
  void reset_accumulation() {
    initialize();

    accumulated_colors.assign(static_cast<std::size_t>(image_width) * image_height,
                              color(0, 0, 0));
    accumulated_samples = 0;
  }

  /** 단일 샘플 패스 계산하기. 최초/카메라변경/해상도변경 시 reset_accumulation() 호출 필요 */
  void render_pass(const hittable& world) {
    for (int j = 0; j < image_height; ++j) {
      for (int i = 0; i < image_width; ++i) {
        const ray r = get_ray(i, j);
        const std::size_t index = static_cast<std::size_t>(j) * image_width + i;

        accumulated_colors[index] += ray_color(r, max_depth, world);
      }
    }
    ++accumulated_samples;
  }

  /** 누적 샘플의 평균색상 반환. (감마보정X) */
  color averaged_color(int i, int j) const {
    if (accumulated_samples == 0)
      return color(0, 0, 0);

    const std::size_t index = static_cast<std::size_t>(j) * image_width + i;

    return accumulated_colors[index] / accumulated_samples;
  }

  void render_progressive(const hittable& world) {
    reset_accumulation();

    for (int sample = 0; sample < samples_per_pixel; ++sample) {
      render_pass(world);

      std::clog << "\rSamples accumulated: " << sample_count() << " / " << samples_per_pixel
                << ' ' << std::flush;
    }

    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    for (int j = 0; j < image_height; ++j) {
      for (int i = 0; i < image_width; ++i) {
        write_color(std::cout, averaged_color(i, j));
      }
    }

    std::clog << "\rDone.                              \n";
  }

private:
  // 카메라 내부 변수 (private)
  int image_height = 0;       // 렌더링된 이미지의 세로축 픽셀 수
  double pixel_samples_scale; // deprecated: 레거시 Bucket Rendering용
  point3 center;              // 카메라 중심
  point3 pixel00_loc;         // 좌상단(가장 처음) 픽셀의 위치
  vec3 pixel_delta_u;         // pixel 간 width축(u축) 거리
  vec3 pixel_delta_v;         // pixel 간 height축(v축) 거리
  vec3 u, v, n;               // 카메라 공간 기저
  vec3 defocus_disk_u;        // defocus disk의 수평 직경
  vec3 defocus_disk_v;        // defocus disk의 수직 직경

  // 점진적 렌더링을 위한 누적변수
  std::vector<color> accumulated_colors; /** (i,j)픽셀은 j*image_width+i에. 감마보정안됨 */
  int accumulated_samples = 0;

  void initialize() {
    image_height = int(image_width / aspect_ratio);
    image_height = (image_height < 1) ? 1 : image_height;

    pixel_samples_scale = 1.0 / samples_per_pixel;

    center = EYE;

    // Viewport의 Dimension
    auto theta = degrees_to_radians(vfov);
    auto h = std::tan(theta / 2);
    auto viewport_height = 2 * h * focus_dist;
    auto viewport_width = viewport_height * (double(image_width) / image_height);

    // u,v,n 기저 계산
    n = unit_vector(EYE - AT);
    u = unit_vector(cross(UP, n));
    v = cross(n, u);

    // Vu, Vv (Image Plane인 Viewport를 따라 좌상단부터 우측아래로 내려가는)
    auto viewport_u = viewport_width * u;
    auto viewport_v = viewport_height * -v;

    // u,v축을 따라 픽셀 간을 이동하는 델타 벡터
    pixel_delta_u = viewport_u / image_width;
    pixel_delta_v = viewport_v / image_height;

    // 좌상단 픽셀 위치 계산
    auto viewport_upper_left = center - (focus_dist * n) - viewport_u / 2 - viewport_v / 2;
    pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

    auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle) / 2);
    defocus_disk_u = u * defocus_radius;
    defocus_disk_v = v * defocus_radius;
  }

  ray get_ray(int i, int j) const {
    // 어떤 위치 i, j에서, origin에서 랜덤 샘플된 지점으로 향하는 ray를 얻는다.
    auto offset = sample_square();
    auto pixel_sample =
        pixel00_loc + ((i + offset.x()) * pixel_delta_u) + ((j + offset.y()) * pixel_delta_v);

    auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
    auto ray_direction = pixel_sample - ray_origin;

    return ray(ray_origin, ray_direction);
  }

  vec3 sample_square() const {
    // [-0.5, -0.5] ~ [0.5, 0.5] unit square에서의 랜덤 지점 뽑기
    return vec3(random_double() - 0.5, random_double() - 0.5, 0);
  }

  point3 defocus_disk_sample() const {
    auto p = random_in_unit_disk();
    return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
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