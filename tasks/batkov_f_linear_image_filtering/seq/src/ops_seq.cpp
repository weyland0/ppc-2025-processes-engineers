#include "batkov_f_linear_image_filtering/seq/include/ops_seq.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "batkov_f_linear_image_filtering/common/include/common.hpp"

namespace batkov_f_linear_image_filtering {

namespace {

std::vector<float> ApplyKernel(const std::array<std::array<float, 3>, 3>& kernel, const Image& image, size_t cy,
                               size_t cx) {
  std::vector<float> result(image.channels, 0);

  for (size_t ky = 0; ky < kernel.size(); ky++) {
    for (size_t kx = 0; kx < kernel.size(); kx++) {
      size_t px = cx + kx - 1;
      size_t py = cy + ky - 1;

      px = std::max<size_t>(0, std::min(px, image.width - 1));
      py = std::max<size_t>(0, std::min(py, image.height - 1));

      size_t pixel_index = ((py * image.width) + px) * image.channels;
      float kernel_value = kernel.at(ky).at(kx);

      for (size_t ch = 0; ch < image.channels; ++ch) {
        result[ch] += static_cast<float>(image.data[pixel_index + ch]) * kernel_value;
      }
    }
  }

  return result;
}

}  // namespace

BatkovFLinearImageFilteringSEQ::BatkovFLinearImageFilteringSEQ(const InType& in) : kernel_() {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = Image{};
}

bool BatkovFLinearImageFilteringSEQ::ValidationImpl() {
  return (!GetInput().data.empty()) && (GetInput().width > 0) && (GetInput().height > 0);
}

bool BatkovFLinearImageFilteringSEQ::PreProcessingImpl() {
  float sum = 0.0F;
  float sigma = 1.0F;

  for (int col = -1; col <= 1; col++) {
    for (int row = -1; row <= 1; row++) {
      kernel_.at(col + 1).at(row + 1) =
          std::exp(-static_cast<float>((row * row) + (col * col)) / (2.0F * sigma * sigma));
      sum += kernel_.at(col + 1).at(row + 1);
    }
  }

  for (size_t col = 0; col < 3; col++) {
    for (size_t row = 0; row < 3; row++) {
      kernel_.at(col).at(row) /= sum;
    }
  }

  GetOutput().width = GetInput().width;
  GetOutput().height = GetInput().width;
  GetOutput().channels = GetInput().channels;
  GetOutput().data.resize(GetInput().width * GetInput().width * GetInput().channels);

  return true;
}

bool BatkovFLinearImageFilteringSEQ::RunImpl() {
  size_t width = GetInput().width;
  size_t height = GetInput().height;
  size_t channels = GetInput().channels;

  size_t block_size = 10;
  for (size_t by = 0; by < height; by += block_size) {
    for (size_t bx = 0; bx < width; bx += block_size) {
      size_t end_y = std::min(by + block_size, height);
      size_t end_x = std::min(bx + block_size, width);

      for (size_t col = by; col < end_y; col++) {
        for (size_t row = bx; row < end_x; row++) {
          auto result = ApplyKernel(kernel_, GetInput(), col, row);

          size_t out_index = (col * width + row) * channels;
          for (size_t ch = 0; ch < channels; ch++) {
            float pixel_val = result[ch];
            GetOutput().data[out_index + ch] = static_cast<uint8_t>(std::max(0.0F, std::min(255.0F, pixel_val)));
          }
        }
      }
    }
  }

  return true;
}

bool BatkovFLinearImageFilteringSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace batkov_f_linear_image_filtering
