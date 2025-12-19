#include "batkov_f_image_smoothing/common/include/gaussian_kernel_fabric.hpp"

#include <cmath>
#include <cstddef>
#include <vector>

namespace batkov_f_image_smoothing {

std::vector<std::vector<float>> GaussianKernelFabric::Create(size_t size, float sigma) {
  std::vector<std::vector<float>> kernel(size, std::vector<float>(size, 0.0F));
  float sum = 0.0F;
  size_t half = size / 2;

  for (size_t i = 0; i < size; i++) {
    for (size_t j = 0; j < size; j++) {
      size_t x = i - half;
      size_t y = j - half;
      float value = exp(-static_cast<float>((x * x) + (y * y)) / (2 * sigma * sigma));
      kernel[i][j] = value;
      sum += value;
    }
  }

  for (size_t i = 0; i < size; i++) {
    for (size_t j = 0; j < size; j++) {
      kernel[i][j] /= sum;
    }
  }

  return kernel;
}

}  // namespace batkov_f_image_smoothing
