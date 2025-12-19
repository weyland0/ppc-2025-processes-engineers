#include <gtest/gtest.h>

#include <memory>

#include "batkov_f_image_smoothing/common/include/common.hpp"
#include "batkov_f_image_smoothing/common/include/smooth_image_detector.hpp"
#include "batkov_f_image_smoothing/mpi/include/ops_mpi.hpp"
#include "batkov_f_image_smoothing/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace batkov_f_image_smoothing {

class BatkovFRunPerfTestProcesses2 : public ppc::util::BaseRunPerfTests<InType, OutType> {
  std::unique_ptr<ISmoothImageDetector> sid_ = std::make_unique<LaplacianVarianceSmoothDetector>();
  InType input_data_;

  void SetUp() override {
    std::string filename = "test1.jpeg";
    std::string abs_path = ppc::util::GetAbsoluteTaskPath(PPC_ID_batkov_f_image_smoothing, filename);

    input_data_ = Image(abs_path);
    sid_->SetThreshold(150.0F);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return sid_->IsSmooth(output_data);
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(BatkovFRunPerfTestProcesses2, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks = ppc::util::MakeAllPerfTasks<InType, BatkovFImageSmoothingMPI, BatkovFImageSmoothingSEQ>(
    PPC_SETTINGS_batkov_f_image_smoothing);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = BatkovFRunPerfTestProcesses2::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, BatkovFRunPerfTestProcesses2, kGtestValues, kPerfTestName);

}  // namespace batkov_f_image_smoothing
