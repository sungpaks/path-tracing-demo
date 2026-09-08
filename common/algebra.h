/*
    algebra.cpp

    Copyright (C) 2013 by Don Cross  -  http://cosinekitty.com/raytrace

    This software is provided 'as-is', without any express or implied
    warranty. In no event will the author be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
       claim that you wrote the original software. If you use this software
       in a product, an acknowledgment in the product documentation would be
       appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
       misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
       distribution.

    -------------------------------------------------------------------------

    Solves algebraic problems for linear systems of 3 equations in 3 unknowns,
    and quadratic, cubic, and quartic equations of one variable.
*/

// Modified from Don Cross's algebra.cpp:
// https://github.com/cosinekitty/raytrace
// Header-only subset, lowercase public API, sorted distinct real roots,
// finite-input validation, and a guard for the cubic triple-root case.

#ifndef ALGEBRA_H
#define ALGEBRA_H

#include <algorithm>
#include <cmath>
#include <complex>
#include <stdexcept>
#include <vector>

namespace algebra {
namespace detail {
using complex = std::complex<double>;
constexpr double TOLERANCE = 1.0e-8;

inline bool IsZero(complex x) {
  return std::abs(x.real()) < TOLERANCE && std::abs(x.imag()) < TOLERANCE;
}

// Returns n=0..2, the number of distinct real roots found for the equation
//
//     ax^2 + bx + c = 0
//
// Stores the roots in the first n slots of the array 'roots'.
inline int SolveQuadraticEquation(complex a, complex b, complex c, complex roots[2]) {
  if (a == complex(0.0, 0.0)) {
    if (IsZero(b)) {
      // The equation devolves to: c = 0, where the variable x has vanished!
      return 0; // cannot divide by zero, so there is no solution.
    } else {
      // Simple linear equation: bx + c = 0, so x = -c/b.
      roots[0] = -c / b;
      return 1; // there is a single solution.
    }
  } else {
    const complex radicand = b * b - 4.0 * a * c;
    if (IsZero(radicand)) {
      // Both roots have the same value: -b / 2a.
      roots[0] = -b / (2.0 * a);
      return 1;
    } else {
      // There are two distinct real roots.
      const complex r = sqrt(radicand);
      const complex d = 2.0 * a;

      roots[0] = (-b + r) / d;
      roots[1] = (-b - r) / d;
      return 2;
    }
  }
}

inline complex cbrt(complex a, int n) {
  /*
            This function returns one of the 3 complex cube roots of the complex number 'a'.
            The value of n=0..2 selects which root is returned.
        */

  const double TWOPI = 2.0 * 3.141592653589793238462643383279502884;

  double rho = pow(abs(a), 1.0 / 3.0);
  double theta = ((TWOPI * n) + arg(a)) / 3.0;
  return complex(rho * cos(theta), rho * sin(theta));
}

// Returns n=0..3, the number of distinct real roots found for the equation
//
//     ax^3 + bx^2 + cx + d = 0
//
// Stores the roots in the first n slots of the array 'roots'.
inline int SolveCubicEquation(complex a, complex b, complex c, complex d, complex roots[3]) {
  if (a == complex(0.0, 0.0)) {
    return SolveQuadraticEquation(b, c, d, roots);
  }

  b /= a;
  c /= a;
  d /= a;

  complex S = b / 3.0;
  complex D = c / 3.0 - S * S;
  complex E = S * S * S + (d - S * c) / 2.0;
  complex Froot = sqrt(E * E + D * D * D);
  complex F = -Froot - E;

  if (IsZero(F)) {
    F = Froot - E;
  }

  if (IsZero(F) && IsZero(D)) {
    roots[0] = -S;
    return 1;
  }

  for (int i = 0; i < 3; ++i) {
    const complex G = cbrt(F, i);
    roots[i] = G - D / G - S;
  }

  return 3;
}

// Returns n=0..4, the number of distinct real roots found for the equation
//
//     ax^4 + bx^3 + cx^2 + dx + e = 0
//
// Stores the roots in the first n slots of the array 'roots'.
inline int SolveQuarticEquation(complex a, complex b, complex c, complex d, complex e,
                                complex roots[4]) {
  if (a == complex(0.0, 0.0)) {
    return SolveCubicEquation(b, c, d, e, roots);
  }

  // See "Summary of Ferrari's Method" in http://en.wikipedia.org/wiki/Quartic_function

  // Without loss of generality, we can divide through by 'a'.
  // Anywhere 'a' appears in the equations, we can assume a = 1.
  b /= a;
  c /= a;
  d /= a;
  e /= a;

  complex b2 = b * b;
  complex b3 = b * b2;
  complex b4 = b2 * b2;

  complex alpha = (-3.0 / 8.0) * b2 + c;
  complex beta = b3 / 8.0 - b * c / 2.0 + d;
  complex gamma = (-3.0 / 256.0) * b4 + b2 * c / 16.0 - b * d / 4.0 + e;

  complex alpha2 = alpha * alpha;
  complex t = -b / 4.0;

  if (IsZero(beta)) {
    complex rad = sqrt(alpha2 - 4.0 * gamma);
    complex r1 = sqrt((-alpha + rad) / 2.0);
    complex r2 = sqrt((-alpha - rad) / 2.0);

    roots[0] = t + r1;
    roots[1] = t - r1;
    roots[2] = t + r2;
    roots[3] = t - r2;
  } else {
    complex alpha3 = alpha * alpha2;
    complex P = -(alpha2 / 12.0 + gamma);
    complex Q = -alpha3 / 108.0 + alpha * gamma / 3.0 - beta * beta / 8.0;
    complex R = -Q / 2.0 + sqrt(Q * Q / 4.0 + P * P * P / 27.0);
    complex U = cbrt(R, 0);
    complex y = (-5.0 / 6.0) * alpha + U;
    if (IsZero(U)) {
      y -= cbrt(Q, 0);
    } else {
      y -= P / (3.0 * U);
    }
    complex W = sqrt(alpha + 2.0 * y);

    complex r1 = sqrt(-(3.0 * alpha + 2.0 * y + 2.0 * beta / W));
    complex r2 = sqrt(-(3.0 * alpha + 2.0 * y - 2.0 * beta / W));

    roots[0] = t + (W - r1) / 2.0;
    roots[1] = t + (W + r1) / 2.0;
    roots[2] = t + (-W - r2) / 2.0;
    roots[3] = t + (-W + r2) / 2.0;
  }

  return 4;
}

} // namespace detail

/**
 * 4차 방정식 해를 구한다. (A*x^4 + B*x^3 + C*x^2 + D*x + E = 0)
 * 오차범위(1e-8) 내의 유한한 실수근을 오름차순으로 반환한다. std::vector<double>
 */
inline std::vector<double> solve_quartic_equation(double A, double B, double C, double D,
                                                  double E) {
  const double coefficients[] = {A, B, C, D, E};
  for (double coefficient : coefficients) {
    if (!std::isfinite(coefficient))
      throw std::invalid_argument("Polynomial coefficients must be finite");
  }

  double leading = 0;
  for (double coefficient : coefficients) {
    if (coefficient != 0) {
      leading = coefficient;
      break;
    }
  }
  if (leading == 0)
    throw std::invalid_argument("The zero polynomial has infinitely many roots");

  // Uniformly scaled equations should produce the same roots.
  detail::complex normalized[5];
  for (int i = 0; i < 5; ++i) {
    const double value = coefficients[i] / leading;
    if (!std::isfinite(value))
      throw std::overflow_error("Polynomial coefficient normalization overflow");
    normalized[i] = value;
  }

  detail::complex candidates[4];
  const int count = detail::SolveQuarticEquation(normalized[0], normalized[1], normalized[2],
                                                 normalized[3], normalized[4], candidates);

  std::vector<double> real_roots;
  for (int i = 0; i < count; ++i) {
    const double real = candidates[i].real();
    const double imag = candidates[i].imag();
    if (!std::isfinite(real) || !std::isfinite(imag))
      throw std::runtime_error("Polynomial solver produced a non-finite root");
    if (std::abs(imag) <= detail::TOLERANCE * std::max(1.0, std::abs(real)))
      real_roots.push_back(real);
  }
  std::sort(real_roots.begin(), real_roots.end());

  std::vector<double> distinct_roots;
  for (double root : real_roots) {
    if (distinct_roots.empty() || std::abs(root - distinct_roots.back()) >
                                      detail::TOLERANCE * std::max(1.0, std::abs(root)))
      distinct_roots.push_back(root);
  }
  return distinct_roots;
}

} // namespace algebra

#endif
