#include <gtest/gtest.h>

#include "batkov_f_vector_sum/common/include/common.hpp"
#include "batkov_f_vector_sum/mpi/include/ops_mpi.hpp"
#include "batkov_f_vector_sum/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace batkov_f_vector_sum {

class BatkovFRunPerfTestProcesses : public ppc::util::BaseRunPerfTests<InType, OutType> {
  InType input_data_;
  OutType expected_sum_;

  void SetUp() override {
    std::string filename = "one_million_vec.txt";
    expected_sum_ = -2609880;

    std::string abs_path = ppc::util::GetAbsoluteTaskPath(PPC_ID_batkov_f_vector_sum, filename);
    std::ifstream file(abs_path);

    if (!file.is_open()) {
      throw std::runtime_error("Failed to open file: " + abs_path);
    }

    int value;
    while (file >> value) {
      input_data_.push_back(value);
    }

    file.close();
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return expected_sum_ == output_data;
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(BatkovFRunPerfTestProcesses, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, BatkovFVectorSumMPI, BatkovFVectorSumSEQ>(PPC_SETTINGS_batkov_f_vector_sum);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = BatkovFRunPerfTestProcesses::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, BatkovFRunPerfTestProcesses, kGtestValues, kPerfTestName);

}  // namespace batkov_f_vector_sum
