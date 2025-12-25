#include "batkov_f_linear_image_filtering/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include "batkov_f_linear_image_filtering/common/include/common.hpp"

namespace batkov_f_linear_image_filtering {

namespace {

float ApplyKernel(const Kernel &kernel, const Image &image, size_t row, size_t col, size_t ch) {
  const size_t width = image.width;
  const size_t height = image.height;
  const size_t channels = image.channels;

  float sum = 0.0F;

  for (size_t ky = 0; ky < 3; ky++) {
    for (size_t kx = 0; kx < 3; kx++) {
      size_t py = row + ky - 1;
      size_t px = col + kx - 1;

      py = std::max<size_t>(py, 0);
      py = std::min<size_t>(py, height - 1);
      px = std::max<size_t>(px, 0);
      px = std::min<size_t>(px, width - 1);

      auto index = (((py * width) + px) * channels) + ch;
      sum += static_cast<float>(image.data[index]) * kernel[ky][kx];
    }
  }

  return sum;
}

void CopyBlockData(const Image &image, Image &block, size_t start_row) {
  for (size_t row = 0; row < block.height; row++) {
    size_t global_row = start_row + row;

    for (size_t col = 0; col < image.width; col++) {
      for (size_t ch = 0; ch < image.channels; ch++) {
        size_t local_index = ((row * image.width + col) * image.channels) + ch;
        size_t global_index = ((global_row * image.width + col) * image.channels) + ch;

        block.data[local_index] = image.data[global_index];
      }
    }
  }
}

void ProcessBlock(const Image &block, Image &result_block, const Kernel &kernel, size_t start_row, size_t local_start) {
  for (size_t row = 0; row < result_block.height; row++) {
    size_t row_in_block = row + (start_row - local_start);

    for (size_t col = 0; col < block.width; col++) {
      for (size_t ch = 0; ch < block.channels; ch++) {
        float val = ApplyKernel(kernel, block, row_in_block, col, ch);
        size_t index = ((row * block.width + col) * block.channels) + ch;
        result_block.data[index] = static_cast<uint8_t>(std::clamp(val, 0.0F, 255.0F));
      }
    }
  }
}

void CopyBlockToOutput(const Image &result_block, Image &output, size_t start_row) {
  for (size_t row = 0; row < result_block.height; row++) {
    for (size_t col = 0; col < result_block.width; col++) {
      for (size_t ch = 0; ch < result_block.channels; ch++) {
        size_t output_index = (((start_row + row) * result_block.width + col) * result_block.channels) + ch;
        size_t block_index = ((row * result_block.width + col) * result_block.channels) + ch;
        output.data[output_index] = result_block.data[block_index];
      }
    }
  }
}

void GatherResultsFromProcesses(size_t mpi_size, size_t rows_per_process, size_t remainder, const Image &result_block,
                                size_t start_row, Image &output) {
  CopyBlockToOutput(result_block, output, start_row);

  for (size_t proc = 1; proc < mpi_size; proc++) {
    size_t p_start = (proc * rows_per_process) + std::min<size_t>(proc, remainder);
    size_t p_end = p_start + rows_per_process + (proc < remainder ? 1 : 0);
    size_t p_rows = p_end - p_start;
    size_t p_data_size = output.width * p_rows * output.channels;

    std::vector<uint8_t> recv_buffer(p_data_size);
    MPI_Recv(recv_buffer.data(), static_cast<int>(p_data_size), MPI_UNSIGNED_CHAR, static_cast<int>(proc), 0,
             MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    for (size_t row = 0; row < p_rows; row++) {
      for (size_t col = 0; col < output.width; col++) {
        for (size_t ch = 0; ch < output.channels; ch++) {
          size_t output_index = (((p_start + row) * output.width + col) * output.channels) + ch;
          size_t recv_buf_index = ((row * output.width + col) * output.channels) + ch;
          output.data[output_index] = recv_buffer[recv_buf_index];
        }
      }
    }
  }
}

}  // namespace

BatkovFLinearImageFilteringMPI::BatkovFLinearImageFilteringMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = Image();
}

bool BatkovFLinearImageFilteringMPI::ValidationImpl() {
  return (!GetInput().data.empty()) && (GetInput().width > 0) && (GetInput().height > 0);
}

bool BatkovFLinearImageFilteringMPI::PreProcessingImpl() {
  kernel_ = {{1.0F / 16.0F, 2.0F / 16.0F, 1.0F / 16.0F},
             {2.0F / 16.0F, 4.0F / 16.0F, 2.0F / 16.0F},
             {1.0F / 16.0F, 2.0F / 16.0F, 1.0F / 16.0F}};

  return true;
}

bool BatkovFLinearImageFilteringMPI::RunImpl() {
  int int_rank = 0;
  int int_size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &int_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &int_size);

  const auto rank = static_cast<size_t>(int_rank);
  const auto size = static_cast<size_t>(int_size);

  auto &input = GetInput();
  size_t width = input.width;
  size_t height = input.height;
  size_t channels = input.channels;

  const size_t kernel_size = 3;
  const size_t half = kernel_size / 2;

  size_t rows_per_process = height / size;
  size_t remainder = height % size;
  size_t start_row = (rank * rows_per_process) + std::min<size_t>(rank, remainder);
  size_t end_row = start_row + rows_per_process + (rank < remainder ? 1 : 0);

  size_t local_start = (start_row > half) ? start_row - half : 0;
  size_t local_end = (end_row + half < height) ? end_row + half : height;

  Image block;
  block.width = width;
  block.height = local_end - local_start;
  block.channels = channels;
  block.data.resize(block.width * block.height * block.channels);
  CopyBlockData(input, block, local_start);

  Image result_block;
  result_block.width = width;
  result_block.height = end_row - start_row;
  result_block.channels = channels;
  result_block.data.resize(result_block.width * result_block.height * result_block.channels);
  ProcessBlock(block, result_block, kernel_, start_row, local_start);

  Image result;
  result.width = width;
  result.height = height;
  result.channels = channels;
  result.data.resize(width * height * channels);

  if (rank == 0) {
    GatherResultsFromProcesses(size, rows_per_process, remainder, result_block, start_row, result);
  } else {
    MPI_Send(result_block.data.data(), static_cast<int>(result_block.data.size()), MPI_UNSIGNED_CHAR, 0, 0,
             MPI_COMM_WORLD);
  }

  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Bcast(result.data.data(), static_cast<int>(result.data.size()), MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
  GetOutput() = std::move(result);

  MPI_Barrier(MPI_COMM_WORLD);
  return true;
}

bool BatkovFLinearImageFilteringMPI::PostProcessingImpl() {
  return true;
}

}  // namespace batkov_f_linear_image_filtering
