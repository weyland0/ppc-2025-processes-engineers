#pragma once

#include <string>
#include <tuple>

#include "batkov_f_image_smoothing/common/include/image.hpp"
#include "task/include/task.hpp"

namespace batkov_f_image_smoothing {

using InType = Image;
using OutType = Image;
using TestType = std::tuple<std::string, float>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace batkov_f_image_smoothing
