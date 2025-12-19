#pragma once

#include <cstddef>
#include <vector>

namespace batkov_f_image_smoothing {

class GaussianKernelFabric {
 public:
  static std::vector<std::vector<float>> Create(size_t size, float sigma);
};

}  // namespace batkov_f_image_smoothing
