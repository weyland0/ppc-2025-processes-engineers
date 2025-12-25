# Линейная фильтрация изображений (блочное разбиение). Ядро Гаусса 3x3.

- **Студент**: Батьков Филипп Владиславович, группа 3823Б1ПР3  
- **Технология**: SEQ | MPI  
- **Вариант**: 28

## 1. Введение

Цель работы — разработать и реализовать линейной фильтраци изображений двумя способами:

- **последовательная (SEQ)** реализация на одном процессе;
- **параллельная (MPI)** реализация с блочным распараллеливанием изображения.

Дополнительно необходимо:

- реализовать вспомогательную инфраструктуру для генерации случайного и детерминированного шума, а также для построения гауссовых ядер;
- реализовать детектор "размытия" изображения на основе дисперсии лапласиана для проверки корректности фильтрации;
- написать функциональные и производительные тесты и сравнить поведение SEQ и MPI реализаций на реальных изображениях.

## 2. Постановка задачи

На вход алгоритма подаётся сгенерируемое программно цветное изображение.

Типы входных и выходных данных:

```cpp
struct Image {
  std::vector<uint8_t> data;
  size_t width;
  size_t height;
  size_t channels;
};

using InType  = Image;  // исходное изображение
using OutType = Image;  // сглаженное изображение
```

Структура `Image` содержит:

- размеры изображения `width`, `height`;
- количество каналов `channels` (1, 3 или 4);
- вектор байтов `std::vector<uint8_t> data`, хранящий пиксели в формате (RGB[A]).

Задача: по входному изображению построить новое изображение, на которое применён **оператор фильтрации** с фиксированным ядром 3×3. Полученное изображение должно быть более чистым, что проверяется при помощи дисперсии Лапласиана.

## 3. Базовый алгоритм (последовательная версия)

Последовательная реализация находится в `seq/src/ops_seq.cpp`, класс `BatkovFLinearImageFilteringSEQ`.

### 3.1. Формирование гауссова ядра

Гауссово ядро размером `3×3` задается прямым образом в функции `PreProcessingImpl`:

```cpp
kernel_ = {{1.0F / 16.0F, 2.0F / 16.0F, 1.0F / 16.0F},
           {2.0F / 16.0F, 4.0F / 16.0F, 2.0F / 16.0F},
           {1.0F / 16.0F, 2.0F / 16.0F, 1.0F / 16.0F}};
```

Это классическое дискретное гауссово ядро, нормированное так, чтобы сумма коэффициентов была равна 1.

### 3.2. Схема свёртки

Алгоритм последовательного сглаживания (`BatkovFLinearImageFilteringSEQ::RunImpl`) выполняет двумерную свёртку исходного изображения с гауссовым ядром:

- извлекаются размеры `width`, `height`, количество каналов `channels` и массив пикселей `img_data`;
- создаётся выходной буфер `temp` размером `width * height * channels`;
- для каждого пикселя `(row, col)` и каждого канала `ch` вычисляется новое значение как взвешенная сумма соседних пикселей в окрестности `3×3`:

```cpp
for (size_t row = 0; row < height; row++) {
for (size_t col = 0; col < width; col++) {
    for (size_t ch = 0; ch < channels; ch++) {
    float val = ApplyKernel(kernel_, GetInput(), row, col, ch);
    size_t index = (((row * width) + col) * channels) + ch;
    GetOutput().data[index] = static_cast<uint8_t>(std::clamp(val, 0.0F, 255.0F));
    }
}
}
```

Границы обрабатываются по схеме **зеркального отражения**: индексы, выходящие за границу, прижимаются к диапазону `[0, width-1]` / `[0, height-1]`.

## 4. Схема распараллеливания (MPI)

Параллельная реализация описана в `mpi/src/ops_mpi.cpp`, класс `BatkovFLinearImageFilteringMPI`.

### 4.1. Общая идея

Распараллеливание выполняется на блоки с определнным количеством строк:

- все процессы знают размеры изображения и входные данные (`GetInput()` одинаков на всех рангах);
- по вертикали (`height`) изображение делится на блоки строк между процессами (почти равные по количеству строк, остаток раздается первым процессам);
- каждый процесс обрабатывает свои строки, но для корректной свёртки с ядром 3×3 ему нужны также **дополнительные строки сверху и снизу** ("halo"-область);
- после локальной обработки каждый процесс отправляет свой фрагмент результата процессу 0, который собирает полное изображение и затем рассылает его всем процессам.

### 4.2. Определение ранга и размеров

В начале `RunImpl` выполняется стандартная инициализация ранга и количетсва процессов MPI:

```cpp
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
```

Затем читаются параметры входного изображения `width`, `height`, `channels` и ссылка на исходные данные.

### 4.3. Разбиение на блоки

Количество строк на процесс и диапазон строк для текущего ранга:

```cpp
size_t rows_per_process = height / size;
size_t remainder        = height % size;

size_t start_row = rank * rows_per_process + std::min<size_t>(rank, remainder);
size_t end_row   = start_row + rows_per_process + (rank < remainder ? 1 : 0);
```

Такой подход гарантирует, что первые `remainder` процессов получат на одну строку больше.

### 4.4. Halo-область для свёртки

Чтобы корректно посчитать свёртку вблизи горизонтальных границ локального блока, каждый процесс расширяет свой диапазон за счёт соседних строк:

```cpp
const size_t kernel_size = 3;
const size_t half        = kernel_size / 2;

size_t local_start  = (start_row > half) ? start_row - half : 0;
size_t local_end    = (end_row + half < height) ? end_row + half : height;
size_t local_height = local_end - local_start;
```

### 4.5. Копирование локальной области

Функция `CopyBlockData` копирует необходимые строки исходного изображения в новый блок:

```cpp
Image block;
block.width = width;
block.height = local_end - local_start;
block.channels = channels;
block.data.resize(block.width * block.height * block.channels);
CopyBlockData(input, block, local_start);
```

Каждая строка `global_row` из диапазона `[local_start, local_end)` копируется в соответствующую строку локального блока.

### 4.6. Локальная свёртка

Функция `ProcessBlock` выполняет свёртку только для строк одного блока `[start_row, end_row)`, используя halo-строки из ранее скопированного `block`:

```cpp
Image result_block;
result_block.width = width;
result_block.height = end_row - start_row;
result_block.channels = channels;
result_block.data.resize(result_block.width * result_block.height * result_block.channels);
ProcessBlock(block, result_block, kernel_, start_row, local_start);
```

Внутри функции логика свёртки полностью аналогична последовательной версии, но индексы `row_in_block` сдвинуты на `local_start`.

### 4.7. Сборка результата на процессе 0

Процесс 0 собирает фрагменты от всех процессов с помощью вспомогательных функций:

```cpp
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
```

Функция `GatherResultsFromProcesses` на процессе 0:

- сначала копирует локальный блок процесса 0 в `result` (`CopyBlockToOutput`);
- затем в цикле по `proc = 1 .. size-1` принимает фрагменты через `MPI_Recv` и раскладывает их на нужные позиции в `result`.

### 4.8. Рассылка результата всем процессам

Чтобы функциональные тесты могли проверять результат на каждом процессе, итоговое изображение рассылается всем рангам через `MPI_Bcast`:

```cpp
MPI_Barrier(MPI_COMM_WORLD);
MPI_Bcast(result.data.data(), static_cast<int>(result.data.size()), MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
GetOutput() = std::move(result);
MPI_Barrier(MPI_COMM_WORLD);
```

Таким образом, после окончания `RunImpl` **все процессы** содержат идентичный результат сглаживания в `GetOutput()`.

## 5. Детали реализации и структура проекта

Структура каталога задачи:

```text
tasks/batkov_f_batkov_f_linear_image_filteringimage_smoothing/
├── common
│   └── include
│       └── common.hpp
├── info.json
├── mpi
│   ├── include
│   │   └── ops_mpi.hpp
│   └── src
│       └── ops_mpi.cpp
├── seq
│   ├── include
│   │   └── ops_seq.hpp
│   └── src
│       └── ops_seq.cpp
├── settings.json
└── tests
    ├── functional
    │   └── main.cpp
    └── performance
        └── main.cpp
```

Основные классы:

- `BatkovFLinearImageFilteringSEQ` — последовательная реализация;
- `BatkovFLinearImageFilteringMPI` — MPI-реализация с блочным разбиением;
- `BatkovFRunFuncTestsProcesses3` — функциональные тесты (сравнение с детектором сглаженности);
- `BatkovFRunPerfTestProcesses3` — тесты производительности.

Функция `CalcLaplacianVariance` используется в обоих тестах для проверки, что изображение действительно стало более чистым.

```cpp
static float CalcLaplacianVariance(const Image& image)
{
  std::vector<float> gray(image.width * image.height);

  const auto &data = image.data;
  size_t width = image.width;
  size_t height = image.height;
  size_t channels = image.channels;

  if (channels == 1) {
    for (size_t i = 0; i < width * height; i++) {
      gray[i] = static_cast<float>(data[i]);
    }
  } else {
    for (size_t i = 0; i < width * height; i++) {
      size_t idx = i * channels;
      auto r = static_cast<float>(data[idx + 0]);
      auto g = static_cast<float>(data[idx + 1]);
      auto b = static_cast<float>(data[idx + 2]);

      gray[i] = (0.299F * r) + (0.587F * g) + (0.114F * b);
    }
  }

  std::vector<float> laplacian(width * height, 0.0F);
  for (size_t y_px = 1; y_px < height - 1; y_px++) {
    for (size_t x_px = 1; x_px < width - 1; x_px++) {
      size_t idx = (y_px * width) + x_px;

      float value = -gray[((y_px - 1) * width) + x_px] - gray[(y_px * width) + (x_px - 1)] + (4.0F * gray[idx]) -
                    gray[(y_px * width) + (x_px + 1)] - gray[((y_px + 1) * width) + x_px];

      laplacian[idx] = value;
    }
  }

  float mean = 0.0F;
  for (size_t i = 0; i < width * height; i++) {
    mean += laplacian[i];
  }
  mean /= static_cast<float>(width * height);

  float variance = 0.0F;
  for (size_t i = 0; i < width * height; i++) {
    float diff = laplacian[i] - mean;
    variance += diff * diff;
  }
  variance /= static_cast<float>(width * height);

  return variance;
}
```

Алгоритм:

1. Перевод изображения в **градации серого** либо берётся единственный канал, либо используется стандартная формула (Y = 0.299 R + 0.587 G + 0.114 B).
2. Вычисление **лапласиана** в каждой внутренней точке (5-точечный шаблон):

```cpp
float value = -gray[(y-1)*width + x] - gray[y*width + (x-1)]
              + 4.0F * gray[idx]
              - gray[y*width + (x+1)] - gray[(y+1)*width + x];
```

3. Подсчёт средней и дисперсии значений лапласиана по всему изображению.

## 6. Экспериментальная среда

| Компонент  | Значение                              |
|-----------|----------------------------------------|
| CPU       | Apple M2 (8 ядер)                      |
| RAM       | 16 GB                                  |
| ОС        | macOS 15.3.1                           |
| Компилятор| g++ (через CMake), стандарт C++20      |
| MPI       | mpirun (Open MPI) 5.0.8                |

Тестовые данные:

1. **Функциональные тесты** (`tests/functional/main.cpp`):
   - изображения с шумом разного размера генерируются случайным образом;
   - для каждого изображения задаётся начальное значение размытия `preprocess_blur_value_`;
   - для каждого теста запускаются обе реализации: SEQ и MPI;
   - в конце еще раз подсчитывается значение размытия и сравнивается с начальным.

2. **Тесты производительности** (`tests/performance/main.cpp`):
   - изображение с детерминированным шумом генерируется на основе хеша;
   - тестовый фреймворк `BaseRunPerfTests` автоматически прогоняет SEQ и MPI-версии в различных режимах запуска (в т.ч. `task_run` и `pipeline`) и для разного числа процессов.

## 7. Результаты и обсуждение

### 7.1. Корректность

- Функциональные тесты проверяют, что результат сглаживания удовлетворяет критерию `(preprocess_blur_value_ / post_process_blur_value) > 2.0F`.
- Для каждого тестового изображения по результатам работы SEQ и MPI реализаций детектор размытия выдаёт одинаковый ответ, что говорит о **функциональной эквивалентности** алгоритмов.

### 7.2. Производительность

**pipeline:**

| Mode | Count | Time, s | Speedup | Efficiency |
| ---- | ----- | ------- | ------- | ---------- |
| SEQ  | 1     | 0.3637  | 1.00    | N/A        |
| MPI  | 1     | 0.4688  | 0.78    | 78.0%      |
| MPI  | 2     | 0.2799  | 1.30    | 65.0%      |
| MPI  | 4     | 0.1823  | 1.99    | 49.8%      |
| MPI  | 8     | 0.2522  | 1.44    | 18.0%      |

**task_run:**

| Mode | Count | Time, s | Speedup | Efficiency |
| ---- | ----- | ------- | ------- | ---------- |
| SEQ  | 1     | 0.3629  | 1.00    | N/A        |
| MPI  | 1     | 0.4571  | 0.79    | 79.3%      |
| MPI  | 2     | 0.2742  | 1.32    | 66.2%      |
| MPI  | 4     | 0.1830  | 1.98    | 49.6%      |
| MPI  | 8     | 0.3299  | 1.10    | 13.7%      |

- При запуске на одном процессе MPI-реализация ожидаемо медленнее SEQ-за счёт накладных расходов на инициализацию MPI и обмен данными.
- При увеличении числа процессов наблюдается уменьшение времени выполнения MPI-версии до определённого предела: строковое разбиение хорошо масштабируется по числу процессов, пока
коммуникационные расходы не начинают преобладать.
- Эффективность распараллеливания сильно зависит от размеров изображения: чем больше пикселей обрабатывает каждый процесс, тем лучше соотношение "вычисления/коммуникации".

## 8. Заключение

В рамках работы реализованы:

1. **Последовательный алгоритм** гауссова сглаживания изображения с ядром 5×5, корректно обрабатывающий границы кадра.
2. **Параллельная MPI-реализация**, использующая разбиение по строкам и halo-область для точного воспроизведения результата свёртки на каждом процессе.
3. **Функциональные и производительные тесты**, демонстрирующие корректность и исследующие поведение алгоритма при разных режимах запуска.

MPI-реализация показывает выигрыш по времени при достаточно больших изображениях и числе процессов, однако эффект ограничивается ростом накладных расходов на синхронизацию и передачу блоков изображения. Тем не менее, предложенный подход легко масштабируется и может быть расширен для более сложных фильтров (большие ядра, последовательность нескольких свёрток) и трёхмерных изображений.

## 10. Источники

1. [Материалы курса](https://learning-process.github.io/parallel_programming_course/ru/common_information/report.html)
2. [Документация Open MPI](https://www.open-mpi.org/doc/)
3. [MPI стандарт](https://www.mpi-forum.org/)
