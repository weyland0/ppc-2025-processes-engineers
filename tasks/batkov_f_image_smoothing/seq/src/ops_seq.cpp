#include "batkov_f_image_smoothing/seq/include/ops_seq.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "batkov_f_image_smoothing/common/include/common.hpp"
#include "batkov_f_image_smoothing/common/include/gaussian_kernel_fabric.hpp"
#include "batkov_f_image_smoothing/common/include/image.hpp"

namespace batkov_f_image_smoothing {

BatkovFImageSmoothingSEQ::BatkovFImageSmoothingSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = Image();

  gaussian_kernel_ = GaussianKernelFabric::Create(5, 1.0F);
}

bool BatkovFImageSmoothingSEQ::ValidationImpl() {
  return (GetInput().GetWidth() > 0) && (GetInput().GetHeight() > 0);
}

bool BatkovFImageSmoothingSEQ::PreProcessingImpl() {
  return true;
}

bool BatkovFImageSmoothingSEQ::RunImpl() {
  auto &img = GetInput();
  size_t width = img.GetWidth();
  size_t height = img.GetHeight();
  size_t channels = img.GetChannels();
  const auto &img_data = img.GetData();

  std::vector<uint8_t> temp(width * height * channels);

  size_t kernel_size = 5;
  size_t half = kernel_size / 2;

  for (size_t y_px = 0; y_px < height; y_px++) {
    for (size_t x_px = 0; x_px < width; x_px++) {
      for (size_t ch = 0; ch < channels; ch++) {
        float value = 0.0F;

        for (size_t ky = 0; ky < kernel_size; ky++) {
          for (size_t kx = 0; kx < kernel_size; kx++) {
            size_t px = x_px + kx - half;
            size_t py = y_px + ky - half;

            px = std::max<size_t>(0, std::min(px, width - 1));
            py = std::max<size_t>(0, std::min(py, height - 1));

            uint8_t pixel_value = img_data[((py * width + px) * channels) + ch];
            value += static_cast<float>(pixel_value) * gaussian_kernel_[ky][kx];
          }
        }

        temp[((y_px * width + x_px) * channels) + ch] = static_cast<uint8_t>(value);
      }
    }
  }

  GetOutput() = Image(temp, width, height, channels);
  return true;
}

bool BatkovFImageSmoothingSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace batkov_f_image_smoothing
