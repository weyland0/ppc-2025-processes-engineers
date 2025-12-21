// #include "batkov_f_linear_image_filtering/mpi/include/ops_mpi.hpp"

// #include <mpi.h>

// #include <numeric>
// #include <vector>

// #include "batkov_f_linear_image_filtering/common/include/common.hpp"
// #include "util/include/util.hpp"

// namespace batkov_f_linear_image_filtering {

//   BatkovFLinearImageFilteringMPI::BatkovFLinearImageFilteringMPI(const InType &in) {
//   SetTypeOfTask(GetStaticTypeOfTask());
//   GetInput() = in;
//   GetOutput() = 0;
// }

// bool BatkovFLinearImageFilteringMPI::ValidationImpl() {
//   return (GetInput() > 0) && (GetOutput() == 0);
// }

// bool BatkovFLinearImageFilteringMPI::PreProcessingImpl() {
//   GetOutput() = 2 * GetInput();
//   return GetOutput() > 0;
// }

// bool BatkovFLinearImageFilteringMPI::RunImpl() {
//   auto input = GetInput();
//   if (input == 0) {
//     return false;
//   }

//   for (InType i = 0; i < GetInput(); i++) {
//     for (InType j = 0; j < GetInput(); j++) {
//       for (InType k = 0; k < GetInput(); k++) {
//         std::vector<InType> tmp(i + j + k, 1);
//         GetOutput() += std::accumulate(tmp.begin(), tmp.end(), 0);
//         GetOutput() -= i + j + k;
//       }
//     }
//   }

//   const int num_threads = ppc::util::GetNumThreads();
//   GetOutput() *= num_threads;

//   int rank = 0;
//   MPI_Comm_rank(MPI_COMM_WORLD, &rank);

//   if (rank == 0) {
//     GetOutput() /= num_threads;
//   } else {
//     int counter = 0;
//     for (int i = 0; i < num_threads; i++) {
//       counter++;
//     }

//     if (counter != 0) {
//       GetOutput() /= counter;
//     }
//   }

//   MPI_Barrier(MPI_COMM_WORLD);
//   return GetOutput() > 0;
// }

// bool BatkovFLinearImageFilteringMPI::PostProcessingImpl() {
//   GetOutput() -= GetInput();
//   return GetOutput() > 0;
// }

// }  // namespace batkov_f_linear_image_filtering
