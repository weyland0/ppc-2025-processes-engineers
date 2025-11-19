#include "batkov_f_vector_sum/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <numeric>
#include <vector>
#include <iostream>

#include "batkov_f_vector_sum/common/include/common.hpp"
#include "util/include/util.hpp"

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

  int mpi_size = 0;
  int rank = 0;

  MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int elements_per_process = GetInput().size() / mpi_size;
  std::vector<int> local_vector(elements_per_process);
  
  MPI_Scatter(
    GetInput().data(),
    elements_per_process,
    MPI_INT,
    local_vector.data(),
    elements_per_process,
    MPI_INT,
    0,
    MPI_COMM_WORLD
  );
  
  int local_sum = std::accumulate(local_vector.begin(), local_vector.end(), 0);  
  
  int global_sum = 0;

  MPI_Allreduce(
    &local_sum,
    &global_sum,
    1,
    MPI_INT,
    MPI_SUM,
    MPI_COMM_WORLD
  );
  
  GetOutput() = global_sum;
  
  return true;
}

bool BatkovFVectorSumMPI::PostProcessingImpl() {
  return true;
}

}  // namespace batkov_f_vector_sum
