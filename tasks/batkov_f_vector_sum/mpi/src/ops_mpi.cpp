#include "batkov_f_vector_sum/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <cstddef>
#include <utility>
#include <vector>

#include "batkov_f_vector_sum/common/include/common.hpp"

namespace batkov_f_vector_sum {

BatkovFVectorSumMPI::BatkovFVectorSumMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool BatkovFVectorSumMPI::ValidationImpl() {
  return GetOutput() == 0;
}

bool BatkovFVectorSumMPI::PreProcessingImpl() {
  return true;
}

bool BatkovFVectorSumMPI::RunImpl() {
  int rank = 0;
  int mpi_size = 0;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

  auto &input = GetInput();
  size_t input_size = input.size();

  size_t base_elements_per_process = input_size / mpi_size;
  size_t extra_elements = input_size % mpi_size;

  bool this_process_gets_extra = std::cmp_less(static_cast<size_t>(rank), extra_elements);
  size_t elements_for_this_process = base_elements_per_process + (this_process_gets_extra ? 1 : 0);

  size_t start_index = 0;
  if (this_process_gets_extra) {
    start_index = (base_elements_per_process * rank) + rank;
  } else {
    start_index = (base_elements_per_process * rank) + extra_elements;
  }
  size_t end_index = start_index + elements_for_this_process;

  int local_sum = 0;
  for (size_t i = start_index; i < end_index && i < input_size; i++) {
    local_sum += input[i];
  }

  int global_sum = 0;
  MPI_Allreduce(&local_sum, &global_sum, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

  GetOutput() = global_sum;
  return true;
}

bool BatkovFVectorSumMPI::PostProcessingImpl() {
  return true;
}

}  // namespace batkov_f_vector_sum
