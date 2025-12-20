#include <gtest/gtest.h>
#include <stb/stb_image.h>

#include <array>
#include <cstddef>
#include <memory>
#include <string>
#include <tuple>

#include "batkov_f_image_smoothing/common/include/common.hpp"
#include "batkov_f_image_smoothing/common/include/image.hpp"
#include "batkov_f_image_smoothing/common/include/smooth_image_detector.hpp"
#include "batkov_f_image_smoothing/mpi/include/ops_mpi.hpp"
#include "batkov_f_image_smoothing/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace batkov_f_image_smoothing {

class BatkovFRunFuncTestsProcesses2 : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    std::string filename = std::get<0>(test_param);
    size_t filename_dot_pos = filename.find_first_of('.');
    filename.at(filename_dot_pos) = '_';

    std::string threshold = std::to_string(std::get<1>(test_param));
    size_t threshold_dot_pos = threshold.find_first_of('.');

    return filename + "_" + threshold.substr(0, threshold_dot_pos);
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    std::string filename = std::get<0>(params);
    float threshold = std::get<1>(params);

    std::string abs_path = ppc::util::GetAbsoluteTaskPath(PPC_ID_batkov_f_image_smoothing, filename);
    input_data_ = Image(abs_path);

    sid_->SetThreshold(threshold);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return sid_->IsSmooth(output_data);
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
  std::unique_ptr<ISmoothImageDetector> sid_ = std::make_unique<LaplacianVarianceSmoothDetector>();
};

namespace {

TEST_P(BatkovFRunFuncTestsProcesses2, ImageSmoothing) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 4> kTestParam = {std::make_tuple("test1.jpeg", 50.0F), std::make_tuple("test2.jpeg", 50.0F),
                                            std::make_tuple("test3.jpeg", 300.0F),
                                            std::make_tuple("test4.jpeg", 110.0F)};

const auto kTestTasksList = std::tuple_cat(
    ppc::util::AddFuncTask<BatkovFImageSmoothingSEQ, InType>(kTestParam, PPC_SETTINGS_batkov_f_image_smoothing),
    ppc::util::AddFuncTask<BatkovFImageSmoothingMPI, InType>(kTestParam, PPC_SETTINGS_batkov_f_image_smoothing));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = BatkovFRunFuncTestsProcesses2::PrintFuncTestName<BatkovFRunFuncTestsProcesses2>;

INSTANTIATE_TEST_SUITE_P(ImageSmoothingTests, BatkovFRunFuncTestsProcesses2, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace batkov_f_image_smoothing
