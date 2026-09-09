#ifndef VIEWER_H
#define VIEWER_H

#include "camera.h"
#include "quaternion.h"

#include <SDL.h>

#include <memory>
#include <stdexcept>
#include <string>

class viewer {
public:
  void run(camera& cam, const hittable& world) {
    cam.reset_accumulation();

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
      throw std::runtime_error(SDL_GetError());

    // 함수 종료 시 SDL 자원 정리.
    // 아래에서 생성하는 texture, renderer, window보다 나중에 해제된다.
    struct sdl_session {
      ~sdl_session() { SDL_Quit(); }
    } session;

    using window_ptr = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>;
    using renderer_ptr = std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)>;
    using texture_ptr = std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>;

    window_ptr window(SDL_CreateWindow("Progressive Path Tracing", SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED, cam.image_width * 2,
                                       cam.height() * 2, SDL_WINDOW_SHOWN),
                      SDL_DestroyWindow);

    if (!window)
      throw std::runtime_error(SDL_GetError());

    renderer_ptr renderer(SDL_CreateRenderer(window.get(), -1, 0), SDL_DestroyRenderer);

    if (!renderer)
      throw std::runtime_error(SDL_GetError());

    texture_ptr texture(SDL_CreateTexture(renderer.get(), SDL_PIXELFORMAT_RGBA32,
                                          SDL_TEXTUREACCESS_STREAMING, cam.image_width,
                                          cam.height()),
                        SDL_DestroyTexture);

    if (!texture)
      throw std::runtime_error(SDL_GetError());

    /** 여기서부터 메인 루프 */
    bool running = true;

    while (running) {
      SDL_Event event;
      bool camera_changed = false;

      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT)
          running = false;
        if (event.type == SDL_KEYDOWN) {
          if (event.key.keysym.sym == SDLK_ESCAPE)
            running = false;
          else if (handle_camera_key(cam, event.key.keysym.sym))
            camera_changed = true;
        }
      }

      if (!running)
        break;
      if (camera_changed)
        cam.reset_accumulation();

      // 1. 전체 픽셀에 대해 샘플링 한 번 진행
      cam.render_pass(world);

      // 2. 현재까지의 평균을 표시용 texture에 기록
      update_texture(texture.get(), cam);

      // 3. 출력할 화면 Clear하고
      if (SDL_RenderClear(renderer.get()) != 0)
        throw std::runtime_error(SDL_GetError());
      // 4. 출력할 화면 준비
      if (SDL_RenderCopy(renderer.get(), texture.get(), nullptr, nullptr) != 0)
        throw std::runtime_error(SDL_GetError());
      // 5. 준비된 화면을 window에 표시
      SDL_RenderPresent(renderer.get());

      const std::string title =
          "Progressive Path Tracing | " + std::to_string(cam.sample_count()) + " spp";

      SDL_SetWindowTitle(window.get(), title.c_str());
    }
  }

private:
  static bool handle_camera_key(camera& cam, SDL_Keycode key) {
    const double move_step = 1;
    const double turn_step = degrees_to_radians(5.0);
    const double pitch_limit = degrees_to_radians(85.0);

    const double target_distance = (cam.AT - cam.EYE).length();

    vec3 forward = unit_vector(cam.AT - cam.EYE);       /** 시선벡터 */
    const vec3 up = unit_vector(cam.UP);                /** 카메라 위쪽 벡터 */
    const vec3 right = unit_vector(cross(forward, up)); /** 카메라 오른쪽 벡터 */

    vec3 movement(0, 0, 0);
    double yaw = 0;
    double pitch = 0;

    switch (key) {

    case SDLK_w:
      movement = move_step * forward;
      break;
    case SDLK_s:
      movement = -move_step * forward;
      break;
    case SDLK_a:
      movement = -move_step * right;
      break;
    case SDLK_d:
      movement = move_step * right;
      break;
    case SDLK_q:
      movement = -move_step * up;
      break;
    case SDLK_e:
      movement = move_step * up;
      break;

    case SDLK_LEFT:
      yaw = turn_step;
      break;
    case SDLK_RIGHT:
      yaw = -turn_step;
      break;
    case SDLK_UP:
      pitch = turn_step;
      break;
    case SDLK_DOWN:
      pitch = -turn_step;
      break;

    default:
      return false;
    }
    if (movement.length_squared() > 0) {
      cam.EYE += movement;
      cam.AT += movement;
      return true;
    }
    if (yaw != 0) { // yaw는 좌우 (고개를 젓듯이)
      forward = quaternion::from_axis_angle(up, yaw).rotate(forward);
    }
    if (pitch != 0) { // pitch는 상하 (고개를 끄덕하듯이)
      // UP과 시선이 평행해지는 일을 방지하기
      const double current_pitch = std::asin(interval(-1.0, 1.0).clamp(dot(forward, up)));

      const double next_pitch =
          interval(-pitch_limit, pitch_limit).clamp(current_pitch + pitch);
      const double actual_pitch = next_pitch - current_pitch;

      if (std::abs(actual_pitch) < 1e-10)
        return false;
      forward = quaternion::from_axis_angle(right, actual_pitch).rotate(forward);
    }
    cam.AT = cam.EYE + target_distance * unit_vector(forward);
    return true;
  }

  /** 선형색상(누적버퍼) --> 표시용 텍스처 데이터타입(RGBA 4바이트 per pixel) */
  static Uint8 to_byte(double linear_component) {
    const double gamma_component = linear_to_gamma(linear_component);
    const interval intensity(0.0, 0.999);

    return static_cast<Uint8>(256 * intensity.clamp(gamma_component));
  }

  static void update_texture(SDL_Texture* texture, const camera& cam) {
    void* pixels = nullptr;
    int pitch = 0;

    if (SDL_LockTexture(texture, nullptr, &pixels, &pitch) != 0)
      throw std::runtime_error(SDL_GetError());

    for (int j = 0; j < cam.height(); ++j) {
      auto* row = static_cast<Uint8*>(pixels) + static_cast<std::size_t>(j) * pitch;
      for (int i = 0; i < cam.image_width; ++i) {
        color c = cam.averaged_color(i, j);
        auto* pixel = row + 4 * i;

        pixel[0] = to_byte(c.x());
        pixel[1] = to_byte(c.y());
        pixel[2] = to_byte(c.z());
        pixel[3] = 255;
      }
    }

    SDL_UnlockTexture(texture);
  }
};

#endif