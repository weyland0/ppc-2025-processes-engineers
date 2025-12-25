#pragma once

#include "batkov_f_linear_image_filtering/common/include/common.hpp"
#include "task/include/task.hpp"

namespace batkov_f_linear_image_filtering {

class BatkovFLinearImageFilteringSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit BatkovFLinearImageFilteringSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

  Kernel kernel_;
};

}  // namespace batkov_f_linear_image_filtering
