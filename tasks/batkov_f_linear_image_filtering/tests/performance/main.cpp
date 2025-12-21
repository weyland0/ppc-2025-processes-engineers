// #include <gtest/gtest.h>

// #include "batkov_f_linear_image_filtering/common/include/common.hpp"
// #include "batkov_f_linear_image_filtering/mpi/include/ops_mpi.hpp"
// #include "batkov_f_linear_image_filtering/seq/include/ops_seq.hpp"
// #include "util/include/perf_test_util.hpp"

// namespace batkov_f_linear_image_filtering {

// class BatkovFRunPerfTestProcesses3 : public ppc::util::BaseRunPerfTests<InType, OutType> {
//   const int kCount_ = 100;
//   InType input_data_{};

//   void SetUp() override {
//     input_data_ = kCount_;
//   }

//   bool CheckTestOutputData(OutType &output_data) final {
//     return input_data_ == output_data;
//   }

//   InType GetTestInputData() final {
//     return input_data_;
//   }
// };

// TEST_P(BatkovFRunPerfTestProcesses3, RunPerfModes) {
//   ExecuteTest(GetParam());
// }

// const auto kAllPerfTasks =
//     ppc::util::MakeAllPerfTasks<InType, BatkovFLinearImageFilteringMPI, BatkovFLinearImageFilteringSEQ>(PPC_SETTINGS_batkov_f_linear_image_filtering);

// const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

// const auto kPerfTestName = BatkovFRunPerfTestProcesses3::CustomPerfTestName;

// INSTANTIATE_TEST_SUITE_P(RunModeTests, BatkovFRunPerfTestProcesses3, kGtestValues, kPerfTestName);

// }  // namespace batkov_f_linear_image_filtering
