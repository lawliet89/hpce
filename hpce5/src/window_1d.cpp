#include "include/window_1d.hpp"
#include <algorithm>


//TODO fri:

// output func [&] or inline directly, per-pixel, advances chunk ptr as required (only forward in chunk sizes)

// make inner loop operate on individual pixels?
// should be fixable +- okish for multibyte
// reading accums and such will still need repacking individual pixels...

// does 1d rolling window min/max over the given chunks of rows and accumulates
// results vertically
void window_1d(uint8_t* const in_buf, uint8_t* const out_buf, uint64_t buf_size,
               const uint32_t chunk_size, const uint32_t img_width_pix,
               const uint32_t img_height, const uint32_t n_levels,
               const uint8_t bit_width)
{
  // TODO: in threaded case, local vals, not static
  static uint32_t i;  // counts bytes

  static uint32_t img_w_bytes;
  static uint64_t extra_chunks;

  static uint64_t chunk_cnt;  // counter
  static uint32_t row_cnt;
  static uint8_t* curr_chunk;

  // window state
  static uint32_t num_windows_assigned;
  static window_state* wss;
  static window_state* ws;

  // TODO: threaded -> done once anyway
  static bool serial_hack = 0;
  if (!serial_hack) {
    //////////////////////////
    // init here
    //////////////////////////
    i = 0;
    img_w_bytes = (img_width_pix * 8) / bit_width;
    uint64_t chunks_per_img =
        ((img_height * img_w_bytes) + chunk_size - 1) / chunk_size;
    uint64_t chunks_per_img_padded =
        (((img_height + n_levels) * img_w_bytes) + chunk_size - 1) / chunk_size;
    extra_chunks = chunks_per_img_padded - chunks_per_img;
    chunk_cnt = 0;
    row_cnt = 0;
    curr_chunk = in_buf;

    // create and initialise rolling windows
    num_windows_assigned = n_levels;
    wss = new window_state[num_windows_assigned];
    for (int m = 0; m < num_windows_assigned; ++m) {
      window_state* ws = &wss[m];
      ws->window_size = (m + 1) * 2 + 1;
      ws->start = new win_queue_entry[ws->window_size];
      ws->q_head = ws->start;

      ws->q_tail = ws->q_head;
      ws->q_head->value = 0xffffffff;  // TODO: max different
      ws->q_head->retire_idx = ws->window_size;
    }

    //////////////////////////
    serial_hack = 1;
  }

  // TODO: flow control with producer here
  // iterate through chunk
  for (int j = 0; j < chunk_size; ++j) {

    uint8_t curr_val = *(curr_chunk + j);

    // TODO
    if (row_cnt >= n_levels) {
      uint8_t* acc0 = curr_chunk + j - (2 * n_levels) * img_w_bytes;
      acc0 += (acc0 < in_buf ? buf_size : 0);
      // fprintf(stderr, "<<<< [%d] thing 2n above: %x\n", ws->window_size,
      // *(acc0));
      fprintf(stderr, "OUT: %2x\n", std::min(*acc0, curr_val));
    }
    // TODO

    // step all windows by 1
    for (int m = 0; m < num_windows_assigned; ++m) {
      ws = &wss[m];

      win_queue_entry* end = ws->start + ws->window_size;
      uint32_t n_depth = (ws->window_size - 1) / 2;
      if (ws->q_head->retire_idx == i) {
        ws->q_head++;
        if (ws->q_head >= end) ws->q_head = ws->start;
      }
      if (curr_val <= ws->q_head->value) {
        ws->q_head->value = curr_val;
        ws->q_head->retire_idx = i + ws->window_size;
        ws->q_tail = ws->q_head;
      } else {
        while (ws->q_tail->value >= curr_val) {
          if (ws->q_tail == ws->start) ws->q_tail = end;
          --(ws->q_tail);
        }
        ++(ws->q_tail);
        if (ws->q_tail == end) ws->q_tail = ws->start;

        ws->q_tail->value = curr_val;
        ws->q_tail->retire_idx = i + ws->window_size;
      }

      uint8_t* acc1 = curr_chunk + j - n_depth * img_w_bytes - n_depth;
      acc1 += (acc1 < in_buf ? buf_size : 0);
      uint8_t* acc2 =
          curr_chunk + j - (2 * n_levels - n_depth) * img_w_bytes - n_depth;
      acc2 += (acc2 < in_buf ? buf_size : 0);

      if (i >= n_depth) {
        ////// out[i] = ws->q_head->value;
        // fprintf(stderr, "[%d] %2d: %x\n", ws->window_size, i,
        // ws->q_head->value);
        // fprintf(stderr, ">> [%d] thing above: %x\n", ws->window_size, *acc1);
        *acc1 = std::min(*acc1, (uint8_t)ws->q_head->value);
        *acc2 = std::min(*acc2, (uint8_t)ws->q_head->value);
        // fprintf(stderr, ">2 [%d] thing above: %x\n", ws->window_size, *acc2);
      }
      // special drain
      if (i == img_w_bytes - 1) {
        for (uint32_t ii = 1; ii <= n_depth; ++ii) {
          // fprintf(stderr, "check for eviction of %d at %d\n",
          // ws->q_head->value, i + ii);
          if (ws->q_head->retire_idx == i + ii) {
            // fprintf(stderr, "evicting: %d at %d\n", ws->q_head->value, i +
            // ii);
            ws->q_head++;
            if (ws->q_head >= end) ws->q_head = ws->start;
          }
          // fprintf(stderr, "[%d] %2d+%2d: %x\n", ws->window_size, i, ii,
          //       ws->q_head->value);
          // fprintf(stderr, ">> [%d] thing above: %x\n", ws->window_size,
          // *(acc1+ii));
          // fprintf(stderr, ">2 [%d] thing above: %x\n", ws->window_size,
          // *(acc2+ii));
          uint8_t* acc1 = curr_chunk + j + ii - n_depth * img_w_bytes - n_depth;
          acc1 += (acc1 < in_buf ? buf_size : 0);
          uint8_t* acc2 = curr_chunk + j + ii -
                          (2 * n_levels - n_depth) * img_w_bytes - n_depth;
          acc2 += (acc2 < in_buf ? buf_size : 0);

          *acc1 = std::min(*acc1, (uint8_t)ws->q_head->value);
          *acc2 = std::min(*acc2, (uint8_t)ws->q_head->value);
        }
        ws->q_tail = ws->q_head;
        ws->q_head->value = 0xffffffff;  // TODO: max different
        ws->q_head->retire_idx = ws->window_size;
      }
    }

    // if at the start of a row, restart the windows' state
    if (i++ == img_w_bytes - 1) {
      i = 0;
      if (++row_cnt == img_height) {

        // finish current chunk
        while (++j < chunk_size) {
          uint8_t* acc0 = curr_chunk + j - (2 * n_levels) * img_w_bytes;
          acc0 += (acc0 < in_buf ? buf_size : 0);

          fprintf(stderr, "OUT FIRSTDRAIN: %2x\n", *acc0);  // TODO: output here
        }
        curr_chunk = (curr_chunk + chunk_size == in_buf + buf_size)
                         ? in_buf
                         : curr_chunk + chunk_size;

        // for any extra chunks: do them (with flow control!)
        for (uint64_t p = 0; p < extra_chunks; ++p) {
          // TODO: flow control for output  here!
          for (uint32_t j = 0; j < chunk_size; ++j) {
            uint8_t* acc0 = curr_chunk + j - (2 * n_levels) * img_w_bytes;
            acc0 += (acc0 < in_buf ? buf_size : 0);

            fprintf(stderr, "OUT: %2x\n", *acc0);  // TODO: output here
          }
          curr_chunk = (curr_chunk + chunk_size == in_buf + buf_size)
                           ? in_buf
                           : curr_chunk + chunk_size;

          // fprintf(stderr, "\nextra chunk done!\n");
        }

        // TODO
        fprintf(stderr, "\nDONE!\n");
        return;
      }
    }
  }
  // done with chunk
  curr_chunk = (curr_chunk + chunk_size == in_buf + buf_size)
                   ? in_buf
                   : curr_chunk + chunk_size;
  // TODO: same for out_buf

  // TODO once: free memory at thread cleanup
}
