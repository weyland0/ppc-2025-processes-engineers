#include "batkov_f_image_smoothing/common/include/image.hpp"

#include <stb_image.h>

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace batkov_f_image_smoothing {

Image::Image(const std::string &filepath) {
  int width = 0;
  int height = 0;
  int channels = 0;

  uint8_t *raw_data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);
  if (raw_data == nullptr) {
    throw std::runtime_error("Не удалось загрузить изображение");
  }

  width_ = static_cast<size_t>(width);
  height_ = static_cast<size_t>(height);
  channels_ = static_cast<size_t>(channels);

  size_t image_size = width_ * height_ * channels_;
  data_.assign(raw_data, raw_data + image_size);

  stbi_image_free(raw_data);
}

Image::Image(const std::vector<uint8_t> &data, size_t width, size_t heigth, size_t channels)
    : width_(width), height_(heigth), channels_(channels), data_(data) {}

size_t Image::GetWidth() const noexcept {
  return width_;
}

size_t Image::GetHeight() const noexcept {
  return height_;
}

size_t Image::GetChannels() const noexcept {
  return channels_;
}

const std::vector<uint8_t> &Image::GetData() const noexcept {
  return data_;
}

}  // namespace batkov_f_image_smoothing
