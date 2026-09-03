# Ray Tracing in One Weekend 학습 프로젝트

첫 학습 코드는 `examples/02_01_ppm/main.cc`에서 시작합니다. 책의 코드를 직접 입력할 수 있도록 해당 파일에는 구현을 넣어 두지 않았습니다.

각 학습 단계는 `examples/<예제 이름>/` 아래에 독립적으로 보존됩니다. 이후 책에서 만드는 헤더도 해당 예제 폴더에 함께 두므로, 나중에 다른 예제의 변경에 영향받지 않고 다시 빌드할 수 있습니다. 예제 이름에는 영문자, 숫자, 밑줄만 사용합니다.

- 온라인 책: <https://raytracing.github.io/books/RayTracingInOneWeekend.html>
- 공식 참고 저장소: <https://github.com/RayTracing/raytracing.github.io/>

## 학습 흐름

```sh
# 보존된 예제 목록
make list

# 이전 단계를 통째로 복사하여 다음 단계 시작
make new NAME=02_02_progress FROM=02_01_ppm

# 특정 예제 Debug 빌드
make build EXAMPLE=02_01_ppm

# 특정 예제 실행(표준출력이 터미널에 표시됨)
make run EXAMPLE=02_01_ppm

# 빌드 후 PPM 생성, 유효성 검사, PNG 변환
make render EXAMPLE=02_01_ppm

# 렌더 후 macOS 이미지 뷰어로 열기
make preview EXAMPLE=02_01_ppm

# 시간이 오래 걸리는 예제의 최적화 렌더
make render-release EXAMPLE=02_01_ppm
```

`EXAMPLE`을 생략하면 기본값은 `02_01_ppm`입니다. 결과는 예제별로 `output/<예제 이름>/image.ppm`과 `image.png`에 저장됩니다. 프로그램의 `std::cout`은 PPM 파일로, `std::clog`/`std::cerr`는 터미널로 분리되므로 책의 진행률 출력도 이미지 파일을 망가뜨리지 않습니다.

VS Code에서는 `Cmd+Shift+B`로 기본 예제를 Debug 빌드할 수 있습니다. Command Palette의 **Tasks: Run Task**에서 기본 예제의 렌더·미리보기·Release 렌더도 선택할 수 있습니다. 다른 예제는 위 명령에서 `EXAMPLE`만 바꾸면 됩니다.

C/C++ 파일은 저장할 때 자동으로 포맷됩니다. 프로젝트의 `.clang-format`은 책과 동일하게 들여쓰기 4칸과 최대 96자 너비를 사용하며, 책의 include 순서와 설명 주석은 임의로 재정렬하지 않습니다.

## 설치된 도구

- Apple Clang: C++ 컴파일러
- CMake + Ninja: Debug/Release 빌드
- LLDB: 디버거
- ImageMagick: PPM 검사 및 PNG 변환
- VS Code C/C++ + CMake Tools: 코드 완성, 오류 표시, 빌드 연동
