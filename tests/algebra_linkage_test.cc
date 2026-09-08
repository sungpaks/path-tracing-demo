#include "algebra.h"

std::vector<double> algebra_roots_from_other_file() {
  return algebra::solve_quartic_equation(1, -10, 35, -50, 24);
}
