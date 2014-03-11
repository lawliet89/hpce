#ifndef WINDOW_1D_H_
#define WINDOW_1D_H_

#include "read_write_sync.hpp"
#include <algorithm>
#include <cstdint>
#include <mutex>

// use of templates -> code in headers

// public
enum {
  MAX_PASS = 0,
  MIN_PASS = 1
};

// private
namespace
{
template <typename T>
struct win_queue_entry
{
  T value;
  uint32_t retire_idx;
};

// state of the sliding window algorithm, packaged in a struct to be able to
// easily do any amount of windows per thread.
template <typename T>
struct window_state
{
  uint32_t window_size;
  win_queue_entry<T>* start;
  win_queue_entry<T>* q_head;
  win_queue_entry<T>* q_tail;
};
}

///////////////////////////////////////////////////////////////////////////////

// Thread slides horizontal 1d windows over streamed input, which are assembled
// vertically into the full SE, reusing window results.
// Does O(n) work per input pixel, windows can be independently in separate
// threads, enabling pixels to be processed in O(1) time.

// handles 1,2,4 byte pixel cases
template <unsigned op_select, typename T>
void window_1d_multibyte(uint8_t* const in_buf_, uint8_t* const out_buf_,
                         const uint64_t buf_size_, const uint32_t chunk_size_,
                         const uint32_t img_width_pix,
                         const uint32_t img_height, const uint32_t n_levels,
                         ReadWriteSync& producer, ReadWriteSync& consumer)
{
  uint32_t chunk_size = chunk_size_ / sizeof(T);
  uint64_t buf_size = buf_size_ / sizeof(T);

  // not a clean way... but helps with main's dynamic binding
  T* in_buf = reinterpret_cast<T*>(in_buf_);
  T* out_buf = reinterpret_cast<T*>(out_buf_);

  uint32_t i;                 // pixels per row counter
  uint32_t row_cnt;           // row counter
  T* curr_chunk;              // ptr to current input chunk
  uint32_t out_subchunk_cnt;  // output pixel counter within a chunk
  T* curr_out_chunk;          // ptr to current output chunk

  // window state
  uint32_t num_windows_assigned;
  window_state<T>* wss;
  window_state<T>* ws;

  std::unique_lock<std::mutex> lock;
  std::unique_lock<std::mutex> resetLock = consumer.waitForReset();

  // initialise counters and buf ptrs
  i = 0;
  row_cnt = 0;
  curr_chunk = in_buf;
  out_subchunk_cnt = 0;
  curr_out_chunk = out_buf;

  // min or max pass (compile time decision)
  auto le_ge_cmp = (op_select == MIN_PASS)
                       ? [](T& lhs, T& rhs) { return lhs <= rhs; }
                       : [](T& lhs, T& rhs) { return lhs >= rhs; };
  auto min_or_max = (op_select == MIN_PASS)
                        ? (const T & (*)(const T&, const T&)) & std::min
                        : (const T & (*)(const T&, const T&)) & std::max;

  uint32_t q_init_val = (op_select == MIN_PASS) ? 0xffffffff : 0x0;

  // create and initialise rolling windows
  num_windows_assigned = n_levels;
  wss = new window_state<T>[num_windows_assigned];
  for (uint32_t m = 0; m < num_windows_assigned; ++m) {
    window_state<T>* ws = &wss[m];
    ws->window_size = (m + 1) * 2 + 1;
    ws->start = new win_queue_entry<T>[ws->window_size];
    ws->q_head = ws->start;

    ws->q_tail = ws->q_head;
    ws->q_head->value = q_init_val;
    ws->q_head->retire_idx = ws->window_size;
  }

  // cleanup to be done before terminating thread
  auto clean_memory = [&wss, &num_windows_assigned]() {
    for (uint32_t m = 0; m < num_windows_assigned; ++m) {
      window_state<T>* ws = &wss[m];
      delete[] ws->start;
    }
    delete[] wss;
  };

  // circular buffer access for chunked buffers (both in and out)
  auto advance_chunk_ptr = [](T*& chunk_ptr, uint32_t chunk_size, T* buf_base,
                              uint64_t buf_size) {
    chunk_ptr = (chunk_ptr + chunk_size == buf_base + buf_size)
                    ? buf_base
                    : chunk_ptr + chunk_size;
  };

  // write one pixel to the output buffer, synchronising with the consumer at
  // output chunk boundaries
  auto output_synced = [&](T value) {
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

  // prepare for production
  lock = consumer.producerWait();

  // main thread loop
  while (1) {
    producer.consumerWait();  // wait for produced chunk

    // if last frame of input sequence, clean up
    if (producer.eof()) {
      consumer.signalEof();
      clean_memory();
      return;
    }

    producer.consume();       // signal consumption of input chunk
    producer.hintProducer();  // wake up producer

    // iterate through input chunk
    for (uint32_t j = 0; j < chunk_size; ++j) {
      T curr_val = *(curr_chunk + j);

      // output vertical accumulator state for the pixel that now has the last
      // dependency (note: no output for first N rows, then one every pixel)
      if (row_cnt >= n_levels) {
        T* acc0 = curr_chunk + j - (2 * n_levels) * img_width_pix;
        acc0 += (acc0 < in_buf ? buf_size : 0);

        output_synced(min_or_max(*acc0, curr_val));
      }

      // step all windows by 1 pixel
      for (uint32_t m = 0; m < num_windows_assigned; ++m) {
        ws = &wss[m];
        win_queue_entry<T>* end = ws->start + ws->window_size;
        uint32_t n_depth = (ws->window_size - 1) / 2;

        if (ws->q_head->retire_idx == i) {
          ws->q_head++;
          if (ws->q_head >= end)
            ws->q_head = ws->start;
        }
        if (le_ge_cmp(curr_val, ws->q_head->value)) {
          ws->q_head->value = curr_val;
          ws->q_head->retire_idx = i + ws->window_size;
          ws->q_tail = ws->q_head;
        } else {
          while (le_ge_cmp(curr_val, ws->q_tail->value)) {
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

        T* acc1 = curr_chunk + j - n_depth * img_width_pix - n_depth;
        acc1 += (acc1 < in_buf ? buf_size : 0);
        T* acc2 =
            curr_chunk + j - (2 * n_levels - n_depth) * img_width_pix - n_depth;
        acc2 += (acc2 < in_buf ? buf_size : 0);

        // update two corresponding vertical accumulators
        // skipping windows that aren't centered within the image
        if (i >= n_depth) {
          *acc1 = min_or_max(*acc1, ws->q_head->value);
          *acc2 = min_or_max(*acc2, ws->q_head->value);
        }
        // at the end of a row, drain window and reset
        if (i == img_width_pix - 1) {
          for (uint32_t ii = 1; ii <= n_depth; ++ii) {
            if (ws->q_head->retire_idx == i + ii) {
              ws->q_head++;
              if (ws->q_head >= end)
                ws->q_head = ws->start;
            }
            T* acc1 = curr_chunk + j + ii - n_depth * img_width_pix - n_depth;
            acc1 += (acc1 < in_buf ? buf_size : 0);
            T* acc2 = curr_chunk + j + ii -
                      (2 * n_levels - n_depth) * img_width_pix - n_depth;
            acc2 += (acc2 < in_buf ? buf_size : 0);

            *acc1 = min_or_max(*acc1, ws->q_head->value);
            *acc2 = min_or_max(*acc2, ws->q_head->value);
          }
          ws->q_tail = ws->q_head;
          ws->q_head->value = q_init_val;
          ws->q_head->retire_idx = ws->window_size;
        }
      }

      // check for end of a row
      if (++i == img_width_pix) {
        i = 0;
        // if done with image input, drain extra N rows of accumulators as
        // they are in the correct state
        if (++row_cnt == img_height) {

          T* acc0 = curr_chunk + j + 1 - (2 * n_levels) * img_width_pix;
          acc0 += (acc0 < in_buf ? buf_size : 0);

          for (uint64_t p = 0; p < img_width_pix * n_levels; ++p) {
            output_synced(*acc0);
            acc0 = (acc0 + 1 == in_buf + buf_size) ? in_buf : acc0 + 1;
          }
          // if ended unaligned with an output chunk boundary, signal
          // production explicitly
          if (out_subchunk_cnt != 0) {
            consumer.produce(std::move(lock));
            lock = consumer.producerWait();
          }

          // reset state at the end of image for next image
          resetLock = consumer.waitForReset();

          // TODO: given time, implement fully correct reset of internal state
          // (not critical for spec)...

          // sync and signal reset up the chain to the producer
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

///////////////////////////////////////////////////////////////////////////////

// 1,2,4 bit cases
template <unsigned op_select, unsigned bit_width>
void window_1d_subbyte(uint8_t* const in_buf, uint8_t* const out_buf,
                       uint64_t buf_size, const uint32_t chunk_size,
                       const uint32_t img_width_pix, const uint32_t img_height,
                       const uint32_t n_levels, ReadWriteSync& producer,
                       ReadWriteSync& consumer)
{
  uint32_t img_w_bytes = (img_width_pix * bit_width) / 8;  // process in bytes

  uint32_t i;                 // pixels per row counter
  uint32_t row_cnt;           // row counter
  uint8_t* curr_chunk;        // ptr to current input chunk
  uint32_t out_subchunk_cnt;  // output pixel counter within a chunk
  uint8_t* curr_out_chunk;    // ptr to current output chunk

  uint8_t subbyte_mask_init;
  uint8_t subbyte_mask_base;

  // window state
  uint32_t num_windows_assigned;
  window_state<uint8_t>* wss;
  window_state<uint8_t>* ws;

  std::unique_lock<std::mutex> lock;
  std::unique_lock<std::mutex> resetLock = consumer.waitForReset();

  // initialise counters and buf ptrs
  i = 0;
  row_cnt = 0;
  curr_chunk = in_buf;
  out_subchunk_cnt = 0;
  curr_out_chunk = out_buf;

  // init subbyte state (1,2,4 bit possibilities)
  subbyte_mask_init =
      (bit_width == 4) ? 0xf0 : ((bit_width == 2) ? 0xc0 : 0x80);
  subbyte_mask_base =
      (bit_width == 4) ? 0x0f : ((bit_width == 2) ? 0x03 : 0x01);

  // min or max pass (compile time decision)
  auto le_ge_cmp = (op_select == MIN_PASS)
                       ? [](uint8_t& lhs, uint8_t& rhs) { return lhs <= rhs; }
                       : [](uint8_t& lhs, uint8_t& rhs) { return lhs >= rhs; };
  auto min_or_max =
      (op_select == MIN_PASS)
          ? (const uint8_t & (*)(const uint8_t&, const uint8_t&)) & std::min
          : (const uint8_t & (*)(const uint8_t&, const uint8_t&)) & std::max;

  uint32_t q_init_val = (op_select == MIN_PASS) ? 0xffffffff : 0x0;

  // create and initialise rolling windows
  num_windows_assigned = n_levels;
  wss = new window_state<uint8_t>[num_windows_assigned];
  for (uint32_t m = 0; m < num_windows_assigned; ++m) {
    window_state<uint8_t>* ws = &wss[m];
    ws->window_size = (m + 1) * 2 + 1;
    ws->start = new win_queue_entry<uint8_t>[ws->window_size];
    ws->q_head = ws->start;

    ws->q_tail = ws->q_head;
    ws->q_head->value = q_init_val;
    ws->q_head->retire_idx = ws->window_size;
  }

  // cleanup to be done before terminating thread
  auto clean_memory = [&wss, &num_windows_assigned]() {
    for (uint32_t m = 0; m < num_windows_assigned; ++m) {
      window_state<uint8_t>* ws = &wss[m];
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

  // write one byte to the output buffer, synchronising with the consumer at
  // output chunk boundaries
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

  // prepare for production
  lock = consumer.producerWait();

  // main thread loop
  while (1) {
    producer.consumerWait();  // wait for produced chunk

    // if last frame of input sequence, clean up
    if (producer.eof()) {
      consumer.signalEof();
      clean_memory();
      return;
    }

    producer.consume();       // signal consumption of input chunk
    producer.hintProducer();  // wake up producer

    // iterate through input chunk in byte steps
    for (uint32_t j = 0; j < chunk_size; ++j) {
      uint8_t curr_val_ = *(curr_chunk + j);

      // iterate through pixels in the byte read above
      uint8_t packing_temp = 0;
      uint8_t subbyte_mask = subbyte_mask_init;
      for (uint32_t t = 0; t < 8 / bit_width; ++t, subbyte_mask >>= bit_width) {
        uint8_t curr_val_unshifted = curr_val_ & subbyte_mask;
        uint8_t curr_val = curr_val_unshifted >> (8 - bit_width * (t + 1));

        // output vertical accumulator state for the pixel that now has the
        // last
        // dependency (note: no output for first N rows, then one every
        // pixel)
        if (row_cnt >= n_levels) {
          uint8_t* acc0 = curr_chunk + j - (2 * n_levels) * img_w_bytes;
          acc0 += (acc0 < in_buf ? buf_size : 0);

          // pack pixel outputs in a byte as they come
          packing_temp |=
              min_or_max(((*acc0) & subbyte_mask), curr_val_unshifted);

          // output entire byte when all sub-pixels are done
          if (t == (8 / bit_width) - 1) {
            output_synced(packing_temp);
          }
        }

        // step all windows by 1 pixel
        for (uint32_t m = 0; m < num_windows_assigned; ++m) {
          ws = &wss[m];
          win_queue_entry<uint8_t>* end = ws->start + ws->window_size;
          uint32_t n_depth = (ws->window_size - 1) / 2;

          if (ws->q_head->retire_idx == i) {
            ws->q_head++;
            if (ws->q_head >= end)
              ws->q_head = ws->start;
          }
          if (le_ge_cmp(curr_val, ws->q_head->value)) {
            ws->q_head->value = curr_val;
            ws->q_head->retire_idx = i + ws->window_size;
            ws->q_tail = ws->q_head;
          } else {
            while (le_ge_cmp(curr_val, ws->q_tail->value)) {
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

          // as pixels are packed within bytes, need to calculate how many bytes
          // and how much to shift by to get to the proper accumulators n_depth
          // pixels to the left (corresponding to the window's middle pixel)
          // note: major headache, should have just considered unpacking
          // explicitly for sub-byte sizes
          uint32_t bytes_back =
              (n_depth + ((8 / bit_width) - 1 - t)) / (8 / bit_width);
          uint32_t acc_sub_pos =
              (n_depth + ((8 / bit_width) - 1 - t)) % (8 / bit_width);

          uint8_t* acc1 = curr_chunk + j - n_depth * img_w_bytes - bytes_back;
          acc1 += (acc1 < in_buf ? buf_size : 0);
          uint8_t* acc2 = curr_chunk + j -
                          (2 * n_levels - n_depth) * img_w_bytes - bytes_back;
          acc2 += (acc2 < in_buf ? buf_size : 0);

          // update two corresponding vertical accumulators
          // skipping windows that aren't centered within the image
          if (i >= n_depth) {
            uint8_t upshifted_q_head = (ws->q_head->value)
                                       << (bit_width * acc_sub_pos);
            uint8_t merge_mask = subbyte_mask_base << (bit_width * acc_sub_pos);

            uint8_t acc1_new =
                min_or_max(((*acc1) & merge_mask), upshifted_q_head);
            uint8_t acc2_new =
                min_or_max(((*acc2) & merge_mask), upshifted_q_head);

            *acc1 = (*acc1 & ~merge_mask) | acc1_new;
            *acc2 = (*acc2 & ~merge_mask) | acc2_new;
          }
          // at the end of a row, drain window and reset
          if (i == img_width_pix - 1) {
            for (uint32_t ii = 1; ii <= n_depth; ++ii) {
              if (ws->q_head->retire_idx == i + ii) {
                ws->q_head++;
                if (ws->q_head >= end)
                  ws->q_head = ws->start;
              }
              // as before, calculate byte and intrabyte offsets for the
              // accumulators to update
              uint32_t bytes_back =
                  (n_depth - ii + ((8 / bit_width) - 1 - t)) / (8 / bit_width);
              uint32_t acc_sub_pos =
                  (n_depth - ii + ((8 / bit_width) - 1 - t)) % (8 / bit_width);

              uint8_t* acc1 =
                  curr_chunk + j - n_depth * img_w_bytes - bytes_back;
              acc1 += (acc1 < in_buf ? buf_size : 0);
              uint8_t* acc2 = curr_chunk + j -
                              (2 * n_levels - n_depth) * img_w_bytes -
                              bytes_back;

              acc2 += (acc2 < in_buf ? buf_size : 0);
              uint8_t upshifted_q_head = (ws->q_head->value)
                                         << (bit_width * acc_sub_pos);
              uint8_t merge_mask = subbyte_mask_base
                                   << (bit_width * acc_sub_pos);

              uint8_t acc1_new =
                  min_or_max(((*acc1) & merge_mask), upshifted_q_head);
              uint8_t acc2_new =
                  min_or_max(((*acc2) & merge_mask), upshifted_q_head);

              *acc1 = (*acc1 & ~merge_mask) | acc1_new;
              *acc2 = (*acc2 & ~merge_mask) | acc2_new;
            }
            ws->q_tail = ws->q_head;
            ws->q_head->value = q_init_val;
            ws->q_head->retire_idx = ws->window_size;
          }
        }

        // check for end of a row
        if (++i == img_width_pix) {
          i = 0;
          // if done with image input, drain extra N rows of accumulators as
          // they are in the correct state
          if (++row_cnt == img_height) {

            uint8_t* acc0 = curr_chunk + j + 1 - (2 * n_levels) * img_w_bytes;
            acc0 += (acc0 < in_buf ? buf_size : 0);

            for (uint64_t p = 0; p < img_w_bytes * n_levels; ++p) {
              output_synced(*acc0);
              acc0 = (acc0 + 1 == in_buf + buf_size) ? in_buf : acc0 + 1;
            }
            // if ended unaligned with an output chunk boundary, signal
            // production explicitly
            if (out_subchunk_cnt != 0) {
              consumer.produce(std::move(lock));
              lock = consumer.producerWait();
            }

            // reset state at the end of image for next image
            resetLock = consumer.waitForReset();

            // TODO: given time, implement fully correct reset of internal state
            // (not critical for spec)...

            // sync and signal reset up the chain to the producer
            consumer.resetDone(std::move(resetLock));
            producer.signalReset();

            // break out of the double loop nest (cleaner way)
            goto break2;
          }
        }
      }
    }
  break2:
    // done with input chunk
    advance_chunk_ptr(curr_chunk, chunk_size, in_buf, buf_size);
  }
}

#endif
