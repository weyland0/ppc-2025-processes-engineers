#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace batkov_f_image_smoothing {

class Image {
 public:
  Image() = default;
  explicit Image(const std::string &filepath);
  explicit Image(const std::vector<uint8_t> &data, size_t width, size_t heigth, size_t channels);
  explicit Image(const std::vector<uint8_t> &&data, size_t width, size_t heigth, size_t channels);

  [[nodiscard]] size_t GetWidth() const noexcept;
  [[nodiscard]] size_t GetHeight() const noexcept;
  [[nodiscard]] size_t GetChannels() const noexcept;
  [[nodiscard]] const std::vector<uint8_t> &GetData() const noexcept;

 private:
  size_t width_ = 0;
  size_t height_ = 0;
  size_t channels_ = 0;
  std::vector<uint8_t> data_;
};

}  // namespace batkov_f_image_smoothing
