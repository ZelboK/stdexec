#define STDEXEC_THROW_ON_CUDA_ERROR

#include <nvexec/stream_context.cuh>
#include <stdexec/execution.hpp>

int main() {
  std::vector<int> input(2048, 1);

  nvexec::stream_context stream{};

  auto snd = stdexec::transfer_just(stream.get_scheduler(), input)
           | nvexec::reduce(std::plus<>{})
           | stdexec::then([] (int i) { return i * 2; });

  auto [result] = stdexec::sync_wait(std::move(snd)).value();

  std::cout << "result: " << result << std::endl;
}
