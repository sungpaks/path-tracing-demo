#include "algebra.h"

#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>

// 여러 소스 파일에서 헤더를 포함해도 링크되는지 확인하기 위해 다른 파일에 정의한다.
std::vector<double> algebra_roots_from_other_file();

void require(bool condition, const std::string& message) {
  if (!condition) {
    std::cerr << "실패: " << message << '\n';
    std::exit(EXIT_FAILURE);
  }
}

void check(const char* name, double a, double b, double c, double d, double e,
           const std::vector<double>& expected) {
  const auto roots = algebra::solve_quartic_equation(a, b, c, d, e);
  require(roots.size() == expected.size(), std::string(name) + ": 근의 개수 확인");
  for (std::size_t i = 0; i < roots.size(); ++i) {
    require(std::abs(roots[i] - expected[i]) <= 1e-7 * std::max(1.0, std::abs(expected[i])),
            std::string(name) + ": 근의 값과 정렬 순서 확인");
    const double x = roots[i];
    const double residual = ((((a * x) + b) * x + c) * x + d) * x + e;
    const double u = std::abs(x);
    const double scale = (((std::abs(a) * u + std::abs(b)) * u + std::abs(c)) * u +
                          std::abs(d)) * u + std::abs(e);
    require(std::abs(residual) <= 1e-7 * std::max(scale, 1e-300),
            std::string(name) + ": 근을 다항식에 대입한 잔차 확인");
  }
}

int main() {
  check("서로 다른 실근 네 개", 1, -10, 35, -50, 24, {1, 2, 3, 4});
  check("양수와 음수가 비대칭으로 섞인 근", 1, -4, -7, 22, 24, {-2, -1, 3, 4});
  check("실근 두 개와 허근 두 개", 1, 0, 0, 0, -1, {-1, 1});
  check("실근이 없는 경우", 1, 0, 0, 0, 1, {});
  check("서로 다른 이중근 두 개", 1, -2, -3, 4, 4, {-1, 2});
  check("사중근 한 개", 1, -4, 6, -4, 1, {1});
  check("근에 0이 포함된 경우", 1, -6, 11, -6, 0, {0, 1, 2, 3});
  check("모든 계수를 작게 조정한 경우", 1e-12, -1e-11, 35e-12, -50e-12, 24e-12,
        {1, 2, 3, 4});
  check("모든 계수를 크게 조정한 경우", 1e12, -1e13, 35e12, -50e12, 24e12,
        {1, 2, 3, 4});
  check("삼차방정식으로 낮아지는 경우", 0, 1, -6, 11, -6, {1, 2, 3});
  check("삼차방정식의 삼중근", 0, 1, -3, 3, -1, {1});
  check("이차방정식으로 낮아지는 경우", 0, 0, 1, -3, 2, {1, 2});
  check("일차방정식으로 낮아지는 경우", 0, 0, 0, 2, -6, {3});
  check("0이 아닌 상수만 남아 해가 없는 경우", 0, 0, 0, 0, 1, {});

  bool rejected_zero = false;
  try {
    algebra::solve_quartic_equation(0, 0, 0, 0, 0);
  } catch (const std::invalid_argument&) {
    rejected_zero = true;
  }
  require(rejected_zero, "모든 계수가 0인 다항식은 예외로 거부해야 함");

  bool rejected_nan = false;
  try {
    algebra::solve_quartic_equation(1, 0, 0, 0, std::numeric_limits<double>::quiet_NaN());
  } catch (const std::invalid_argument&) {
    rejected_nan = true;
  }
  require(rejected_nan, "NaN 계수는 예외로 거부해야 함");
  require(algebra_roots_from_other_file().size() == 4, "다른 소스 파일에서의 함수 호출 확인");
  std::cout << "모든 방정식 풀이 테스트를 통과했습니다.\n";
}
