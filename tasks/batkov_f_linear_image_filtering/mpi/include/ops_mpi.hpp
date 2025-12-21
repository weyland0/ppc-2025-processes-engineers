// #pragma once

// #include "batkov_f_linear_image_filtering/common/include/common.hpp"
// #include "task/include/task.hpp"

// namespace batkov_f_linear_image_filtering {

// class BatkovFLinearImageFilteringMPI : public BaseTask {
//  public:
//   static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
//     return ppc::task::TypeOfTask::kMPI;
//   }
//   explicit BatkovFLinearImageFilteringMPI(const InType &in);

//  private:
//   bool ValidationImpl() override;
//   bool PreProcessingImpl() override;
//   bool RunImpl() override;
//   bool PostProcessingImpl() override;
// };

// }  // namespace batkov_f_linear_image_filtering
