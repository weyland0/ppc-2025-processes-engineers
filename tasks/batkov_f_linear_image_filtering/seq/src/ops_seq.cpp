#include "batkov_f_linear_image_filtering/seq/include/ops_seq.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>

#include "batkov_f_linear_image_filtering/common/include/common.hpp"

namespace batkov_f_linear_image_filtering {

namespace {

float ApplyKernel(const Kernel &kernel, const Image &image, size_t row, size_t col, size_t ch) {
  const size_t width = image.width;
  const size_t height = image.height;
  const size_t channels = image.channels;

  float sum = 0.0F;

  for (size_t ky = 0; ky < 3; ky++) {
    for (size_t kx = 0; kx < 3; kx++) {
      size_t py = row + ky - 1;
      size_t px = col + kx - 1;

      py = std::max<size_t>(py, 0);
      py = std::min<size_t>(py, height - 1);
      px = std::max<size_t>(px, 0);
      px = std::min<size_t>(px, width - 1);

      auto index = (((py * width) + px) * channels) + ch;
      sum += static_cast<float>(image.data[index]) * kernel[ky][kx];
    }
  }

  return sum;
}

}  // namespace

BatkovFLinearImageFilteringSEQ::BatkovFLinearImageFilteringSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = Image{};
}

bool BatkovFLinearImageFilteringSEQ::ValidationImpl() {
  return (!GetInput().data.empty()) && (GetInput().width > 0) && (GetInput().height > 0);
}

bool BatkovFLinearImageFilteringSEQ::PreProcessingImpl() {
  kernel_ = {{1.0F / 16.0F, 2.0F / 16.0F, 1.0F / 16.0F},
             {2.0F / 16.0F, 4.0F / 16.0F, 2.0F / 16.0F},
             {1.0F / 16.0F, 2.0F / 16.0F, 1.0F / 16.0F}};

  GetOutput().width = GetInput().width;
  GetOutput().height = GetInput().height;
  GetInput().channels = GetInput().channels;
  GetOutput().data.resize(GetInput().width * GetInput().height * GetInput().channels);

  return true;
}

bool BatkovFLinearImageFilteringSEQ::RunImpl() {
  size_t width = GetInput().width;
  size_t height = GetInput().height;
  size_t channels = GetInput().channels;

  for (size_t row = 0; row < height; row++) {
    for (size_t col = 0; col < width; col++) {
      for (size_t ch = 0; ch < channels; ch++) {
        float val = ApplyKernel(kernel_, GetInput(), row, col, ch);
        size_t index = (((row * width) + col) * channels) + ch;
        GetOutput().data[index] = static_cast<uint8_t>(std::clamp(val, 0.0F, 255.0F));
      }
    }
  }

  return true;
}

bool BatkovFLinearImageFilteringSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace batkov_f_linear_image_filtering
