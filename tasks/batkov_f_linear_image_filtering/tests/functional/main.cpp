#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <random>
#include <string>
#include <tuple>
#include <vector>

#include "batkov_f_linear_image_filtering/common/include/common.hpp"
#include "batkov_f_linear_image_filtering/mpi/include/ops_mpi.hpp"
#include "batkov_f_linear_image_filtering/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace batkov_f_linear_image_filtering {

class BatkovFRunFuncTestsProcesses3 : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    std::string p0 = std::get<0>(test_param);
    std::string p1 = std::to_string(std::get<1>(test_param));
    std::string p2 = std::to_string(std::get<2>(test_param));
    std::string p3 = std::to_string(std::get<3>(test_param));
    return p0 + "_" + p1 + "x" + p2 + "x" + p3;
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    input_data_.width = std::get<1>(params);
    input_data_.height = std::get<2>(params);
    input_data_.channels = std::get<3>(params);

    size_t size = input_data_.width * input_data_.height * input_data_.channels;
    input_data_.data.resize(size);

    for (size_t i = 0; i < size; ++i) {
      input_data_.data[i] = dis_(gen_);
    }
  }

  static float CalculateMSE(const Image& original, const Image& filtered) {    
    float sum = 0.0F;
    for (size_t i = 0; i < original.data.size(); i++) {
        float diff = static_cast<float>(original.data[i]) - static_cast<float>(filtered.data[i]);
        sum += diff * diff;
    }
    
    return sum / static_cast<float>(original.data.size());
  }

  static float CalculatePSNR(const Image& original, const Image& filtered) {
    float mse = CalculateMSE(original, filtered);
    
    if (mse < 1e-10) {
        return 100.0;
    }
    
    float max_value = 255.0F;
    float psnr = 10.0F * std::log10f((max_value * max_value) / mse);
    
    return psnr;
}

  bool CheckTestOutputData(OutType &output_data) final {
    if (input_data_.data.size() != output_data.data.size()) {
      return false;
    }

    float psnr = CalculatePSNR(input_data_, output_data);
    std::cout << "psnr = " << psnr << '\n';

    return psnr > 40.0F;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  std::random_device rd_;
  std::mt19937 gen_{rd_()};
  std::uniform_int_distribution<size_t> dis_{0, 255};

  InType input_data_;
};

namespace {

TEST_P(BatkovFRunFuncTestsProcesses3, ImageSmoothing) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 4> kTestParam = {
    std::make_tuple("tiny_image", 10, 10, 3), std::make_tuple("small_image", 50, 50, 3),
    std::make_tuple("medium_image", 100, 100, 3), std::make_tuple("big_image", 300, 300, 3)};

const auto kTestTasksList = std::tuple_cat(
    ppc::util::AddFuncTask<BatkovFLinearImageFilteringSEQ, InType>(kTestParam, PPC_SETTINGS_batkov_f_linear_image_filtering));
    // ppc::util::AddFuncTask<BatkovFImageSmoothingMPI, InType>(kTestParam, PPC_SETTINGS_batkov_f_image_smoothing));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = BatkovFRunFuncTestsProcesses3::PrintFuncTestName<BatkovFRunFuncTestsProcesses3>;

INSTANTIATE_TEST_SUITE_P(ImageSmoothingTests, BatkovFRunFuncTestsProcesses3, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace batkov_f_linear_image_filtering