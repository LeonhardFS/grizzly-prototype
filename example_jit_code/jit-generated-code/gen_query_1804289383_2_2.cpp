#include "data_types.h"
#include "iostream"
#include "runtime/JitDispatcher.h"
#include "runtime/JitRuntime.h"
#include "runtime/Variant.hpp"
#include "runtime/jit_global_state.hpp"
#include "string.h"
#include "tbb/atomic.h"
static Dispatcher* dispatcher;
const int64_t window_size1 = 10;
const int64_t window_buffers1 = 2;
record1** state1;
GlobalState* globalState;
Variant* variant;

void pipeline0(size_t currentWindow, int thread_id, int numa_node) {
  for (size_t i = 0; i < 999996; i++) {
    auto key = i;
    auto record = state1[currentWindow][i];
    if (record.count != 0)
      record.price_avg = ((double)record.price_sum) / ((double)record.count);
    std::cout << key << ":" << record.price_avg << "|" << record.price_sum
              << "|" << record.count << "|" << std::endl;
  }
}

void pipeline1(record0* records, size_t size, int thread_id, int numa_node) {
  auto window_state = globalState->window_state[1];
  ThreadLocalState* thread_local_state =
      window_state->thread_local_state[thread_id];
  for (size_t i = 0; i < size; ++i) {
    record0& record = records[i];
    if (record.auction >= 50) {
      int64_t ts = time(NULL);

      if (ts >=
          thread_local_state->windowEnds[thread_local_state->current_window]) {
        size_t old_window = thread_local_state->current_window;
        // change the window state of this thread -> so from now on it will put
        // tuple to the next window
        thread_local_state->windowEnds[old_window] +=
            (window_size1 * window_buffers1);
        thread_local_state->current_window = (old_window + 1) % window_buffers1;
        int64_t oldCount =
            window_state->global_tigger_counter.fetch_and_increment();
        if (oldCount == dispatcher->parallelism - 1) {
          window_state->global_tigger_counter = 0;
          pipeline0(old_window, thread_id, numa_node);
          record1 t;
          std::fill(state1[old_window], state1[old_window] + 999996, t);
        }
      }
      size_t window_index = thread_local_state->current_window;
      auto keyField = record.auction;
      uint64_t value = (uint64_t)keyField;
      if (value < 59 || value > 999996) {
        throw DeoptimizeException(1, i, records);
      }
      uint64_t key = value % 999996;
      auto bufferIndex = window_index;
      auto recordValue = record.price;
      state1[bufferIndex][key].count++;
      state1[bufferIndex][key].price_sum += recordValue;
    }
  }
}

void open(GlobalState* g, Dispatcher* d, Variant* v) {
  globalState = g;
  dispatcher = d;
  variant = v;
  {  // init state buffer for shared aggregation
    auto stateBuffers = window_buffers1 * 1;
    state1 = new record1*[stateBuffers];
    for (size_t w = 0; w < (stateBuffers); w++) {
      state1[w] = new record1[999996 + 1];
      for (size_t i = 0; i < 999996 + 1; i++) {
        state1[w][i] = {};
      }
    }
  }
}

void init(GlobalState* g, Dispatcher* d) {
  {
    g->window_state[1] = new WindowState{};
    auto window_state = g->window_state[1];
    window_state->thread_local_state =
        new ThreadLocalState*[dispatcher->parallelism];
    size_t ts = time(NULL);
    for (size_t thread_ID = 0; thread_ID < dispatcher->parallelism;
         thread_ID++) {
      window_state->thread_local_state[thread_ID] = new ThreadLocalState{};
      window_state->thread_local_state[thread_ID]->windowEnds =
          new int64_t[window_buffers1];
      for (size_t w = 0; w < window_buffers1; w++) {
        ;
        window_state->thread_local_state[thread_ID]->windowEnds[w] =
            ts + (10 * w) + 10;
      }
    }
  }
}

void close() {}

void migrateFrom(void** inputStates) {
  {
    tbb::concurrent_unordered_map<long, record1>* input =
        ((tbb::concurrent_unordered_map<long, record1>*)inputStates[1]);
    for (size_t w = 0; w < (window_buffers1); w++) {
      for (size_t n = 0; n < 1; n++) {
        for (auto const& it : input[w]) {
          long key = it.first;
          record1 record = it.second;
          auto keyValue = ((uint64_t)key);
          uint64_t keyInt = keyValue % 999996;
          state1[w + n][keyInt].price_avg =
              state1[w + n][keyInt].price_avg + record.price_avg;
          state1[w + n][keyInt].price_sum =
              state1[w + n][keyInt].price_sum + record.price_sum;
          state1[w + n][keyInt].count =
              state1[w + n][keyInt].count + record.count;
        }
      }
    }
  }
}

void migrateTo(void** outputStates) {
  {
    tbb::concurrent_unordered_map<long, record1>* output =
        ((tbb::concurrent_unordered_map<long, record1>*)outputStates[1]);
    for (size_t w = 0; w < (window_buffers1); w++) {
      for (size_t n = 0; n < 1; n++) {
        for (int i = 0; i < 999996; i++) {
          long key = i;
          record1 record = state1[w * n][i];
          output[w][key].price_avg =
              output[w][key].price_avg + record.price_avg;
          output[w][key].price_sum =
              output[w][key].price_sum + record.price_sum;
          output[w][key].count = output[w][key].count + record.count;
        }
      }
    }
  }
}

void execute(int threadID, int numaNode) {
  while (dispatcher->hasWork() && variant->isValid()) {
    void* records = dispatcher->getWork(threadID, 0);
    pipeline1((record0*)records, dispatcher->runLength, threadID, numaNode);
    variant->runtime->monitor(threadID);
  }
}

void** getState() {
  void** statePtr = (void**)malloc(sizeof(void*) * 2);
  return statePtr;
}
