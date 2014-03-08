#ifndef WINDOW_1D_H_
#define WINDOW_1D_H_
#include <cstdint>
#include "read_write_sync.hpp"

#include <cstdint>

// TODO: out_buf
// TODO: remove bitwidth via specialisation
// TODO: condition var+mutex argument?
/*template <unsigned MIN_WINDOW>
void window_1d_min(uint8_t* const in_buf, uint8_t* const out_buf,
                   uint64_t buf_size, const uint32_t chunk_size,
                   const uint32_t img_width_pix, const uint32_t img_height_pix,
                   const uint32_t n_levels, const uint8_t bit_width,
                   ReadWriteSync& producer, ReadWriteSync& consumer);
*/

#include <algorithm>
#include <cstdint>
#include <mutex>

// TODO: temp
#include <iostream>

enum {MIN_S, MAX_S};

namespace
{
struct win_queue_entry
{
  uint32_t value;
  uint32_t retire_idx;
};

struct window_state
{
  uint32_t window_size;
  win_queue_entry* start;
  win_queue_entry* q_head;
  win_queue_entry* q_tail;
};
}

// TODO
// make inner loop operate on individual pixels?
// should be fixable +- okish for multibyte
// reading accums and such will still need repacking individual pixels...

// does 1d rolling window min/max over the given chunks of rows and accumulates
// results vertically
template <unsigned MIN_WINDOW, typename T>
void window_1d_min(T* const in_buf, uint8_t* const out_buf, uint64_t buf_size,
                   const uint32_t chunk_size, const uint32_t img_width_pix,
                   const uint32_t img_height, const uint32_t n_levels,
                   const uint8_t bit_width, ReadWriteSync& producer,
                   ReadWriteSync& consumer)
{
  try
  {
    uint32_t i;  // counts pixels per row

    uint32_t img_w_bytes;

    uint32_t row_cnt;
    uint8_t* curr_chunk;  // TODO: multibyte version

    uint32_t out_subchunk_cnt;
    uint8_t* curr_out_chunk;  // TODO: multibyte version

    // window state
    uint32_t num_windows_assigned;
    window_state* wss;
    window_state* ws;

    std::unique_lock<std::mutex> lock;
    std::unique_lock<std::mutex> resetLock = consumer.waitForReset();

    // initialise counters and compute chunk padding for extra N rows
    i = 0;

    img_w_bytes = (img_width_pix * 8) / bit_width;

    row_cnt = 0;
    curr_chunk = in_buf;

    out_subchunk_cnt = 0;
    curr_out_chunk = out_buf;

    auto le_ge_cmp =
        (MIN_WINDOW) ? [](uint8_t& lhs, uint32_t& rhs) { return lhs <= rhs; }
                     : [](uint8_t& lhs, uint32_t& rhs) { return lhs >= rhs; };
    auto min_or_max =
        (MIN_WINDOW)
            ? (const uint8_t & (*)(const uint8_t&, const uint8_t&)) & std::min
            : (const uint8_t & (*)(const uint8_t&, const uint8_t&)) & std::max;

    uint32_t q_init_val = (MIN_WINDOW) ? 0xffffffff : 0x0;

    // create and initialise rolling windows
    num_windows_assigned = n_levels;
    wss = new window_state[num_windows_assigned];
    for (int m = 0; m < num_windows_assigned; ++m) {
      window_state* ws = &wss[m];
      ws->window_size = (m + 1) * 2 + 1;
      ws->start = new win_queue_entry[ws->window_size];
      ws->q_head = ws->start;

      ws->q_tail = ws->q_head;
      ws->q_head->value = q_init_val;
      ws->q_head->retire_idx = ws->window_size;
    }

    // cleanup to be done before terminating thread
    auto clean_memory = [&wss, &num_windows_assigned]() {
      for (int m = 0; m < num_windows_assigned; ++m) {
        window_state* ws = &wss[m];
        delete[] ws->start;
      }
      delete[] wss;
    };

    // circular buffer access for chunked buffers (both in and out)
    auto advance_chunk_ptr = [](uint8_t*& chunk_ptr, uint32_t chunk_size,
                                uint8_t* buf_base, uint64_t buf_size) {
      chunk_ptr = (chunk_ptr + chunk_size == buf_base + buf_size)
                      ? buf_base
                      : chunk_ptr + chunk_size;
    };

    // write one pixel to the output buffer, synchronising with the consumer at
    // consumer chunk boundaries
    auto output_synced = [&](uint8_t value) {
      *(curr_out_chunk + out_subchunk_cnt) = value;
      if (++out_subchunk_cnt == chunk_size) {
        consumer.produce(std::move(lock));
        lock = consumer.producerWait();
        advance_chunk_ptr(curr_out_chunk, chunk_size, out_buf, buf_size);
        out_subchunk_cnt = 0;
      }
    };

    // synchronisation barrier, all threads in chain need to be ready before
    // reading first chunk
    consumer.resetDone(std::move(resetLock));
    producer.signalReset();

    lock = consumer.producerWait();

    // main thread loop
    while (1) {
      producer.consumerWait();

      // if last frame of input sequence, clean up
      if (producer.eof()) {
        consumer.signalEof();
        clean_memory();
        return;
      }

      producer.consume();
      producer.hintProducer();

      // iterate through chunk
      for (int j = 0; j < chunk_size; ++j) {  // TODO multibyte
        uint8_t curr_val = *(curr_chunk + j);

        // output vertical accumulator now that the bottom pixel of the unrolled
        // diamond is available
        if (row_cnt >= n_levels) {
          uint8_t* acc0 = curr_chunk + j - (2 * n_levels) * img_w_bytes;
          acc0 += (acc0 < in_buf ? buf_size : 0);

          output_synced(min_or_max(*acc0, curr_val));
        }

        // step all windows by 1 pixel
        for (int m = 0; m < num_windows_assigned; ++m) {
          ws = &wss[m];
          win_queue_entry* end = ws->start + ws->window_size;
          uint32_t n_depth = (ws->window_size - 1) / 2;

          if (ws->q_head->retire_idx == i) {
            ws->q_head++;
            if (ws->q_head >= end)
              ws->q_head = ws->start;
          }
          // if (curr_val <= ws->q_head->value) {
          if (le_ge_cmp(curr_val, ws->q_head->value)) {
            ws->q_head->value = curr_val;
            ws->q_head->retire_idx = i + ws->window_size;
            ws->q_tail = ws->q_head;
          } else {
            while (le_ge_cmp(curr_val, ws->q_tail->value)) {
              // while (curr_val <= ws->q_tail->value) {
              if (ws->q_tail == ws->start)
                ws->q_tail = end;
              --(ws->q_tail);
            }
            ++(ws->q_tail);
            if (ws->q_tail == end)
              ws->q_tail = ws->start;

            ws->q_tail->value = curr_val;
            ws->q_tail->retire_idx = i + ws->window_size;
          }

          uint8_t* acc1 = curr_chunk + j - n_depth * img_w_bytes - n_depth;
          acc1 += (acc1 < in_buf ? buf_size : 0);
          uint8_t* acc2 =
              curr_chunk + j - (2 * n_levels - n_depth) * img_w_bytes - n_depth;
          acc2 += (acc2 < in_buf ? buf_size : 0);

          // do not process windows that aren't centered within the image
          if (i >= n_depth) {
            *acc1 = min_or_max(*acc1, (uint8_t)ws->q_head->value);
            *acc2 = min_or_max(*acc2, (uint8_t)ws->q_head->value);
          }
          // special drain
          if (i == img_width_pix - 1) {
            for (uint32_t ii = 1; ii <= n_depth; ++ii) {
              if (ws->q_head->retire_idx == i + ii) {
                ws->q_head++;
                if (ws->q_head >= end)
                  ws->q_head = ws->start;
              }
              uint8_t* acc1 =
                  curr_chunk + j + ii - n_depth * img_w_bytes - n_depth;
              acc1 += (acc1 < in_buf ? buf_size : 0);
              uint8_t* acc2 = curr_chunk + j + ii -
                              (2 * n_levels - n_depth) * img_w_bytes - n_depth;
              acc2 += (acc2 < in_buf ? buf_size : 0);

              *acc1 = min_or_max(*acc1, (uint8_t)ws->q_head->value);
              *acc2 = min_or_max(*acc2, (uint8_t)ws->q_head->value);
            }
            ws->q_tail = ws->q_head;
            ws->q_head->value = q_init_val;
            ws->q_head->retire_idx = ws->window_size;
          }
        }

        // if at the start of a row, restart the windows' state
        if (++i == img_width_pix) {
          i = 0;
          // if done with image input
          if (++row_cnt == img_height) {

            uint8_t* acc0 = curr_chunk + j + 1 - (2 * n_levels) * img_w_bytes;
            acc0 += (acc0 < in_buf ? buf_size : 0);
            for (uint64_t p = 0; p < img_w_bytes * n_levels; ++p) {
              output_synced(*acc0);
              acc0 = (acc0 + 1 == in_buf + buf_size) ? in_buf : acc0 + 1;
            }
            if (out_subchunk_cnt != 0) {
              consumer.produce(std::move(lock));
              lock = consumer.producerWait();
            }

            // reset state at the end of image for next image
            resetLock = consumer.waitForReset();

            // TODO: given time, implement fully correct reset...

            consumer.resetDone(std::move(resetLock));
            producer.signalReset();

            break;
          }
        }
      }
      // done with input chunk
      advance_chunk_ptr(curr_chunk, chunk_size, in_buf, buf_size);
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "[Min] Caught exception : " << e.what() << "\n";
    return;
  }
}

void window_1d_max(uint8_t* const in_buf, uint8_t* const out_buf,
                   uint64_t buf_size, const uint32_t chunk_size,
                   const uint32_t img_width_pix, const uint32_t img_height_pix,
                   const uint32_t n_levels, const uint8_t bit_width,
                   ReadWriteSync& producer, ReadWriteSync& consumer);
#endif
