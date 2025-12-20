#pragma once

#include <cstddef>

#include "batkov_f_image_smoothing/common/include/image.hpp"

namespace batkov_f_image_smoothing {

class ISmoothImageDetector {
 public:
  virtual ~ISmoothImageDetector() = default;

  virtual void SetThreshold(float) noexcept = 0;
  [[nodiscard]] virtual bool IsSmooth(const Image &) const = 0;
};

class LaplacianVarianceSmoothDetector : public ISmoothImageDetector {
 public:
  ~LaplacianVarianceSmoothDetector() override = default;

  void SetThreshold(float threshold) noexcept override;
  [[nodiscard]] bool IsSmooth(const Image &image) const override;

 private:
  [[nodiscard]] static float CalcLaplacianVariance(const Image &image);

  float threshold_ = 100;
};

}  // namespace batkov_f_image_smoothing
