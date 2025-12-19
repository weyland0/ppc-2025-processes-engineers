#include "batkov_f_image_smoothing/common/include/smooth_image_detector.hpp"

#include <algorithm>
#include <cstddef>
#include <vector>

#include "batkov_f_image_smoothing/common/include/image.hpp"

namespace batkov_f_image_smoothing {

void LaplacianVarianceSmoothDetector::SetThreshold(float threshold) noexcept {
  threshold_ = std::max(threshold_, threshold);
}

bool LaplacianVarianceSmoothDetector::IsSmooth(const Image& image) const {
  float blur_value = LaplacianVarianceSmoothDetector::CalcLaplacianVariance(image);
  return blur_value < threshold_;
}

float LaplacianVarianceSmoothDetector::CalcLaplacianVariance(const Image& image) {
  std::vector<float> gray(image.GetWidth() * image.GetHeight());

  const auto& image_data = image.GetData();
  size_t width = image.GetWidth();
  size_t height = image.GetHeight();
  size_t channels = image.GetChannels();

  if (image.GetChannels() == 1) {
    for (size_t i = 0; i < width * height; i++) {
      gray[i] = static_cast<float>(image_data[i]);
    }
  } else {
    for (size_t i = 0; i < width * height; i++) {
      size_t idx = i * channels;
      auto r = static_cast<float>(image_data[idx + 0]);
      auto g = static_cast<float>(image_data[idx + 1]);
      auto b = static_cast<float>(image_data[idx + 2]);

      gray[i] = 0.299F * r + 0.587F * g + 0.114F * b;
    }
  }

  std::vector<float> laplacian(width * height, 0.0F);
  for (size_t y_px = 1; y_px < height - 1; y_px++) {
    for (size_t x_px = 1; x_px < width - 1; x_px++) {
      size_t idx = (y_px * width) + x_px;

      float value = -gray[((y_px - 1) * width) + x_px] - gray[(y_px * width) + (x_px - 1)] + (4.0F * gray[idx]) -
                    gray[(y_px * width) + (x_px + 1)] - gray[((y_px + 1) * width) + x_px];

      laplacian[idx] = value;
    }
  }

  float mean = 0.0F;
  for (size_t i = 0; i < width * height; i++) {
    mean += laplacian[i];
  }
  mean /= static_cast<float>(width * height);

  float variance = 0.0F;
  for (size_t i = 0; i < width * height; i++) {
    float diff = laplacian[i] - mean;
    variance += diff * diff;
  }
  variance /= static_cast<float>(width * height);

  return variance;
}

}  // namespace batkov_f_image_smoothing
