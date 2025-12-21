#include "batkov_f_linear_image_filtering/seq/include/ops_seq.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "batkov_f_linear_image_filtering/common/include/common.hpp"

namespace batkov_f_linear_image_filtering {

namespace {

Pixel ApplyKernel(const std::vector<std::vector<float>>& kernel, const Image& image, size_t cy, size_t cx) {
  float r_ch = 0.0F;
  float g_ch = 0.0F;
  float b_ch = 0.0F;

  for (size_t ky = 0; ky < kernel.size(); ky++) {
    for (size_t kx = 0; kx < kernel.size(); kx++) {
      size_t px = cx + kx - 1;
      size_t py = cy + ky - 1;

      px = std::max<size_t>(0, std::min(px, image.width - 1));
      py = std::max<size_t>(0, std::min(py, image.height - 1));

      size_t pixel_index = (py * image.width) + px;
      float kernel_value = kernel[ky][kx];

      r_ch += static_cast<float>(image.data[pixel_index].r) * kernel_value;
      g_ch += static_cast<float>(image.data[pixel_index].g) * kernel_value;
      b_ch += static_cast<float>(image.data[pixel_index].b) * kernel_value;
    }
  }

  Pixel result;
  result.r = static_cast<uint8_t>(r_ch);
  result.g = static_cast<uint8_t>(g_ch);
  result.b = static_cast<uint8_t>(b_ch);

  return result;
}

}  // namespace

BatkovFLinearImageFilteringSEQ::BatkovFLinearImageFilteringSEQ(const InType& in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = Image{};
}

bool BatkovFLinearImageFilteringSEQ::ValidationImpl() {
  return (!GetInput().data.empty()) && (GetInput().width > 0) && (GetInput().height > 0);
}

bool BatkovFLinearImageFilteringSEQ::PreProcessingImpl() {
  kernel_.resize(3);
  for (auto& v : kernel_) {
    v.resize(3);
  }
  
  float sum = 0.0F;
  float sigma = 1.0F;

  for (int col = -1; col <= 1; col++) {
    for (int row = -1; row <= 1; row++) {
      kernel_[col + 1][row + 1] =
          std::exp(-static_cast<float>((row * row) + (col * col)) / (2.0F * sigma * sigma));
      sum += kernel_[col + 1][row + 1];
    }
  }

  for (size_t col = 0; col < 3; col++) {
    for (size_t row = 0; row < 3; row++) {
      kernel_[col][row] /= sum;
    }
  }

  GetOutput().width = GetInput().width;
  GetOutput().height = GetInput().height;
  GetOutput().data.resize(GetInput().width * GetInput().height);

  return true;
}

bool BatkovFLinearImageFilteringSEQ::RunImpl() {
  size_t width = GetInput().width;
  size_t height = GetInput().height;
  
  for (size_t col = 0; col < height; col++) {
    for (size_t row = 0; row < width; row++) {
      auto result = ApplyKernel(kernel_, GetInput(), col, row);
      GetOutput().data[((col * width) + row)] = result;
    }
  }

  return true;
}

bool BatkovFLinearImageFilteringSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace batkov_f_linear_image_filtering
